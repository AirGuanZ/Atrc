#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/render_target.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/renderer_interactor.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/create/renderer.h>
#include <agz/tracer/render/vol_bidir_path_tracing.h>
#include <agz/tracer/utility/parallel_grid.h>
#include <agz/utility/thread.h>

AGZ_TRACER_BEGIN

namespace
{
    struct AtomicSpectrum
    {
        std::atomic<real> channels[SPECTRUM_COMPONENT_COUNT];

        AtomicSpectrum() noexcept
        {
            for(auto &c : channels)
                c = real(0);
        }

        AtomicSpectrum(const AtomicSpectrum &s) noexcept
        {
            for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
                channels[i] = s.channels[i].load();
        }

        void add(const Spectrum &s) noexcept
        {
            for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
                math::atomic_add(channels[i], s[i]);
        }

        Spectrum to_spectrum() const noexcept
        {
            Spectrum ret;
            for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
                ret[i] = channels[i];
            return ret;
        }
    };
}

class VolBDPTRenderer : public Renderer
{
public:

    explicit VolBDPTRenderer(const VolBDPTRendererParams &params);

    RenderTarget render(
        FilmFilterApplier filter, Scene &scene,
        RendererInteractor &reporter) override;

private:

    using ImageBuffer = ImageBufferTemplate<true, true, true, true, true>;

    using ParticleImage = Image2D<AtomicSpectrum>;

    using FilmGridView = FilmFilterApplier::FilmGridView<
        Spectrum, real, Spectrum, Vec3, real>;

    struct EvalPathParams
    {
        const Scene &scene;
        FilmGridView &film_grid_view;
        ParticleImage &particle_image;
        FilmFilterApplier filter;

        Vec2 full_res;

        Rect2 particle_sample_pixel_bound;
        Rect2i particle_pixel_range;

        render::vol_bdpt::Vertex *camera_subpath_space = nullptr;
        render::vol_bdpt::Vertex *light_subpath_space  = nullptr;
    };

    template<bool USE_MIS>
    int render_bdpt_path(
        EvalPathParams &params,
        int px, int py,
        NativeSampler &sampler, Arena &arena);

    template<bool USE_MIS>
    int render_grid(
        const Scene &scene, NativeSampler &sampler,
        FilmGridView &film_grid_view, ParticleImage &particle_image,
        FilmFilterApplier filter, int spp);

    template<bool REPORT_WITH_PREVIEW, bool USE_MIS>
    RenderTarget render_impl(
        FilmFilterApplier filter, Scene &scene, RendererInteractor &reporter);

    VolBDPTRendererParams params_;
};

template<bool USE_MIS>
int VolBDPTRenderer::render_bdpt_path(
    EvalPathParams &params,
    int px, int py,
    NativeSampler &sampler, Arena &arena)
{
    // sample film coord

    const Sample2 &film_sam = sampler.sample2();

    const Vec2 pixel_coord = {
        px + film_sam.u,
        py + film_sam.v
    };

    const Vec2 film_coord = {
        pixel_coord.x / params.full_res.x,
        pixel_coord.y / params.full_res.y
    };

    // generate camera ray

    const auto cam_sam = params.scene.get_camera()->sample_we(
        film_coord, sampler.sample2());
    const Ray cam_ray(cam_sam.pos_on_cam, cam_sam.pos_to_out);

    // build camera/light subpath

    const auto camera_subpath = build_camera_subpath(
        params_.cam_max_vtx_cnt, cam_ray, params.scene,
        sampler, arena, params.camera_subpath_space);

    const auto select_light = params.scene.sample_light(sampler.sample1());
    if(!select_light.light)
        return 0;

    const auto light_subpath = build_light_subpath(
        params_.lht_max_vtx_cnt, select_light, params.scene,
        sampler, arena, params.light_subpath_space);

    render::vol_bdpt::EvalBDPTPathParams path_params = {
        params.scene,
        params.particle_sample_pixel_bound,
        params.full_res,
        sampler
    };

    int ret = 0;

    const Spectrum radiance = render::vol_bdpt::eval_bdpt_path<USE_MIS>(
        path_params,
        camera_subpath.vertices, camera_subpath.vertex_count,
        light_subpath.vertices, light_subpath.vertex_count,
        select_light, [&](const Vec2 &particle_coord, const Spectrum &rad)
    {
        if(rad.is_finite())
        {
            apply_image_filter(
                params.particle_pixel_range, params.filter.radius(),
                particle_coord, [&](int pix, int piy, real rel_x, real rel_y)
            {
                const real weight = params.filter.eval_filter(rel_x, rel_y);
                params.particle_image(piy, pix).add(weight * rad);
            });
            ret = 1;
        }
    });

    if(radiance.is_finite())
    {
        params.film_grid_view.apply(
            pixel_coord.x, pixel_coord.y,
            radiance, 1,
            camera_subpath.g_albedo,
            camera_subpath.g_normal,
            camera_subpath.g_denoise);
    }

    return ret;
}

template<bool USE_MIS>
int VolBDPTRenderer::render_grid(
    const Scene &scene, NativeSampler &sampler,
    FilmGridView &film_grid_view, ParticleImage &particle_image,
    FilmFilterApplier filter, int spp)
{
    if(scene.lights().empty())
        return 0;

    std::vector<render::vol_bdpt::Vertex> cam_subpath(params_.cam_max_vtx_cnt);
    std::vector<render::vol_bdpt::Vertex> lht_subpath(params_.lht_max_vtx_cnt);

    const Rect2 particle_sample_pixel_bound = {
        { 0, 0 },
        { real(filter.width() - 1), real(filter.height() - 1) }
    };

    const Rect2i particle_pixel_range = {
        { 0, 0 },
        { filter.width() - 1, filter.height() - 1 }
    };

    EvalPathParams eval_params = {
        scene,
        film_grid_view,
        particle_image,
        filter,
        { real(filter.width()), real(filter.height()) },
        particle_sample_pixel_bound,
        particle_pixel_range,
        cam_subpath.data(),
        lht_subpath.data()
    };

    int particle_count = 0;

    Arena arena;

    const Rect2i sample_pixels = film_grid_view.sample_pixels();

    for(int py = sample_pixels.low.y; py <= sample_pixels.high.y; ++py)
    {
        for(int px = sample_pixels.low.x; px <= sample_pixels.high.x; ++px)
        {
            for(int i = 0; i < spp; ++i)
            {
                particle_count += render_bdpt_path<USE_MIS>(
                    eval_params, px, py, sampler, arena);

                if(arena.used_bytes() >= 32 * 1024 * 1024)
                    arena.release();

                if(stop_rendering_)
                    return particle_count;
            }
        }
    }

    return particle_count;
}

template<bool REPORT_WITH_PREVIEW, bool USE_MIS>
RenderTarget VolBDPTRenderer::render_impl(
    FilmFilterApplier filter, Scene &scene, RendererInteractor &reporter)
{
    // initialize image buffers

    ImageBuffer image_buffer(filter.width(), filter.height());
    ParticleImage particle_image(filter.height(), filter.width());

    std::atomic<uint64_t> particle_count = 0;

    // thread pool

    const int thread_count = thread::actual_worker_count(params_.worker_count);
    thread::thread_group_t threads(thread_count);

    // per-thread samplers

    Arena sampler_arena;
    auto sampler_prototype = newBox<NativeSampler>(42, false);
    std::vector<NativeSampler *> perthread_samplers;
    for(int i = 0; i < thread_count; ++i)
    {
        perthread_samplers.push_back(
            sampler_prototype->clone(i, sampler_arena));
    }

    // reporter

    reporter.begin();
    reporter.new_stage();

    std::mutex reporter_mutex;

    // do the real work

    if constexpr(REPORT_WITH_PREVIEW)
    {
        // 1. render 1 spp for fast previewing
        // 2. divide remaining spp(s) into tasks, then divide tasks into
        //    iterations. sync all threads per iteration and update reporter

        // previewing image computation

        auto get_img = [&]
        {
            const auto fwd_ratio = image_buffer.weight.map([](real w)
            {
                return w > 0 ? 1 / w : real(0);
            });
            const auto fwd_img = fwd_ratio * image_buffer.value;

            const real bwd_ratio = filter.width() * filter.height() *
                (particle_count > 0 ? real(1) / particle_count : real(0));
            const auto bwd_img = particle_image.map([&](const AtomicSpectrum &as)
            {
                return bwd_ratio * as.to_spectrum();
            });

            return fwd_img + bwd_img;
        };

        // render 1 spp for fast previewing

        parallel_for_2d_grid(
            thread_count,
            filter.width(), filter.height(),
            params_.task_grid_size, params_.task_grid_size,
            threads, [&](int thread_index, const Rect2i &grid)
        {
            auto view = filter.create_subgrid_view({
                grid.low, grid.high - Vec2i(1)},
                image_buffer.value, image_buffer.weight,
                image_buffer.albedo,
                image_buffer.normal,
                image_buffer.denoise);

            const int delta_pc = render_grid<USE_MIS>(
                scene, *perthread_samplers[thread_index],
                view, particle_image, filter, 1);

            particle_count += delta_pc;

            return !stop_rendering_;
        });

        reporter.progress(100.0 / params_.spp, get_img);

        // render remaining spp

        const int preview_spp_interval = (std::max)(1, (params_.spp - 1) / 25);

        int finished_spp = 1;
        while(finished_spp < params_.spp)
        {
            if(stop_rendering_)
                break;

            const int new_finished_spp = (std::min)(
                params_.spp, finished_spp + preview_spp_interval);
            const int delta_spp = new_finished_spp - finished_spp;

            parallel_for_2d_grid(
                thread_count,
                filter.width(), filter.height(),
                params_.task_grid_size, params_.task_grid_size,
                threads, [&](int thread_index, const Rect2i &grid)
            {
                auto view = filter.create_subgrid_view({
                    grid.low, grid.high - Vec2i(1) },
                    image_buffer.value, image_buffer.weight,
                    image_buffer.albedo,
                    image_buffer.normal,
                    image_buffer.denoise);

                const int delta_pc = render_grid<USE_MIS>(
                    scene, *perthread_samplers[thread_index],
                    view, particle_image, filter, delta_spp);

                particle_count += delta_pc;

                return !stop_rendering_;
            });

            finished_spp = new_finished_spp;
            const double progress_percent = 100.0 * finished_spp / params_.spp;

            std::lock_guard lock(reporter_mutex);
            reporter.progress(progress_percent, get_img);
        }
    }
    else
    {
        // simply dividie film into grids (tasks)

        const int total_pixel_count = filter.width() * filter.height();
        std::atomic<int> finished_pixel_count = 0;

        parallel_for_2d_grid(
            thread_count,
            filter.width(), filter.height(),
            params_.task_grid_size, params_.task_grid_size,
            threads, [&](int thread_index, const Rect2i &grid)
        {
            if(stop_rendering_)
                return false;

            auto view = filter.create_subgrid_view({
                grid.low, grid.high - Vec2i(1) },
                image_buffer.value, image_buffer.weight,
                image_buffer.albedo,
                image_buffer.normal,
                image_buffer.denoise);

            const int delta_pc = render_grid<USE_MIS>(
                scene, *perthread_samplers[thread_index],
                view, particle_image, filter, params_.spp);

            particle_count += delta_pc;

            finished_pixel_count += (grid.high - grid.low).product();

            const double percent = 100.0 * finished_pixel_count
                                         / total_pixel_count;

            {
                std::lock_guard lock(reporter_mutex);
                reporter.progress(percent, {});
            }

            return true;
        });
    }

    // reporter

    reporter.end_stage();
    reporter.end();

    // forward image

    RenderTarget render_target;

    const auto fwd_ratio = image_buffer.weight.map([](real w)
    {
        return w > 0 ? 1 / w : real(0);
    });
    render_target.image   = image_buffer.value   * fwd_ratio;
    render_target.albedo  = image_buffer.albedo  * fwd_ratio;
    render_target.normal  = image_buffer.normal  * fwd_ratio;
    render_target.denoise = image_buffer.denoise * fwd_ratio;

    // backward image

    const real bwd_ratio = filter.width() * filter.height() *
        (particle_count > 0 ? real(1) / particle_count : real(0));
    render_target.image += particle_image.map(
        [&](const AtomicSpectrum &as)
    {
        return bwd_ratio * as.to_spectrum();
    });

    return render_target;
}

VolBDPTRenderer::VolBDPTRenderer(const VolBDPTRendererParams &params)
    : params_(params)
{
    
}

RenderTarget VolBDPTRenderer::render(
    FilmFilterApplier filter, Scene &scene, RendererInteractor &reporter)
{
    if(params_.use_mis)
    {
        if(reporter.need_image_preview())
            return render_impl<true, true>(filter, scene, reporter);
        return render_impl<false, true>(filter, scene, reporter);
    }

    if(reporter.need_image_preview())
        return render_impl<true, false>(filter, scene, reporter);
    return render_impl<false, false>(filter, scene, reporter);
}

RC<Renderer> create_vol_bdpt_renderer(const VolBDPTRendererParams &params)
{
    return newRC<VolBDPTRenderer>(params);
}

AGZ_TRACER_END
