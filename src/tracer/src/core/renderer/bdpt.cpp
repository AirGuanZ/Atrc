#include <atomic>
#include <mutex>
#include <thread>

#include <agz/tracer/core/renderer_interactor.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/create/renderer.h>
#include <agz/tracer/render/bidir_path_tracing.h>
#include <agz/utility/thread.h>

AGZ_TRACER_BEGIN

class BDPTRenderer : public Renderer
{
public:

    explicit BDPTRenderer(const BDPTRendererParams &params);

    RenderTarget render(
        FilmFilterApplier filter, Scene &scene,
        RendererInteractor &reporter) override;

private:

    using FilmGrid = FilmFilterApplier::FilmGrid<
        Spectrum, real, Spectrum, Vec3, real>;
    using ParticleFilm = FilmFilterApplier::FilmGridView<Spectrum>;

    struct GridParams
    {
        const Scene  &scene;
        Sampler      &sampler;
        FilmGrid     &film_grid;
        ParticleFilm &particle_film;
        const Vec2   &full_res;
        render::bdpt::BDPTVertex *cam_subpath_space = nullptr;
        render::bdpt::BDPTVertex *lht_subpath_space = nullptr;
    };

    template<bool REPORTER_WITH_PREVIEW>
    RenderTarget render_impl(
        FilmFilterApplier filter, Scene &scene, RendererInteractor &reporter);

    int render_grid(
        const Scene &scene, Sampler &sampler, FilmGrid &film_grid,
        ParticleFilm &particle_film, const Vec2i &full_res);

    void eval_sample(
        int px, int py, const GridParams &grid_params, Arena &arena);

    render::bdpt::BDPTParams algo_params_;

    BDPTRendererParams params_;
};

BDPTRenderer::BDPTRenderer(const BDPTRendererParams &params)
    : params_(params)
{
    algo_params_.max_cam_vtx_cnt = params_.cam_max_vtx_cnt;
    algo_params_.max_lht_vtx_cnt = params_.lht_max_vtx_cnt;
}

RenderTarget BDPTRenderer::render(
    FilmFilterApplier filter, Scene &scene, RendererInteractor &reporter)
{
    if(reporter.need_image_preview())
        return render_impl<true>(filter, scene, reporter);
    return render_impl<false>(filter, scene, reporter);
}

template<bool REPORTER_WITH_PREVIEW>
RenderTarget BDPTRenderer::render_impl(
    FilmFilterApplier filter, Scene &scene, RendererInteractor &reporter)
{
    reporter.begin();
    reporter.new_stage();

    const int width = filter.width();
    const int height = filter.height();
    const int x_task_count = (width + params_.task_grid_size - 1)
                           / params_.task_grid_size;
    const int y_task_count = (height + params_.task_grid_size - 1)
                           / params_.task_grid_size;
    const int total_task_count = x_task_count * y_task_count;

    std::mutex reporter_mutex;
    std::atomic<int> next_task_id = 0;

    std::atomic<uint64_t> total_particle_count = 0;

    const int worker_count = thread::actual_worker_count(params_.worker_count);
    auto particle_images_lock = newBox<std::mutex[]>(worker_count);

    std::vector<Image2D<Spectrum>> particle_output_film;
    if constexpr(REPORTER_WITH_PREVIEW)
    {
        particle_output_film.resize(
            worker_count, Image2D<Spectrum>(filter.height(), filter.width()));
    }

    ImageBufferTemplate<true, true, true, true, true> image_buffer(width, height);

    auto render_func = [
        &filter,
        &image_buffer,
        &scene,
        &reporter,
        &reporter_mutex,
        &next_task_id,
        &total_particle_count,
        worker_count,
        &particle_images_lock,
        &particle_output_film,
        x_task_count,
        total_task_count,
        task_grid_size = params_.task_grid_size,
        this
    ] (Sampler *sampler, Image2D<Spectrum> *particle_image, int i)
    {
        const Vec2i full_res = { filter.width(), filter.height() };
        ParticleFilm particle_film = filter.create_subgrid_view(
            { { 0, 0 }, { filter.width() - 1, filter.height() - 1 } },
            *particle_image);

        for(;;)
        {
            if(stop_rendering_)
                break;

            const int task_id = next_task_id++;
            if(task_id >= total_task_count)
                break;

            const int grid_y_index = task_id / x_task_count;
            const int grid_x_index = task_id % x_task_count;
            
            const int x_beg = grid_x_index * task_grid_size;
            const int y_beg = grid_y_index * task_grid_size;
            const int x_end = (std::min)(x_beg + task_grid_size, filter.width());
            const int y_end = (std::min)(y_beg + task_grid_size, filter.height());

            FilmGrid film_grid = filter.create_subgrid<
                Spectrum, real, Spectrum, Vec3, real>(
                    { { x_beg, y_beg }, { x_end - 1, y_end - 1 } });

            int task_particle_count = this->render_grid(
                scene, *sampler, film_grid, particle_film, full_res);

            if constexpr(REPORTER_WITH_PREVIEW)
            {
                uint64_t pc;

                {
                    std::lock_guard lk(particle_images_lock[i]);
                    particle_output_film[i] = *particle_image;

                    total_particle_count += task_particle_count;
                    pc = total_particle_count;
                }

                auto get_img = [
                    &image_buffer,
                    worker_count,
                    &particle_images_lock,
                    &particle_output_film,
                    &filter,
                    pc]()
                {
                    Image2D<Spectrum> ret(filter.height(), filter.width());

                    for(int j = 0; j < worker_count; ++j)
                    {
                        std::lock_guard lk(particle_images_lock[j]);
                        ret = ret + particle_output_film[j];
                    }

                    real ratio_particle = filter.width() * filter.height()
                                        * (pc ? real(1) / pc : real(0));
                    ret = ratio_particle * ret;

                    auto ratio_forward = image_buffer.weight.map([](real w)
                    {
                        return w > 0 ? 1 / w : real(0);
                    });
                    return ret + ratio_forward * image_buffer.value;
                };

                const real percent = real(100) * (task_id + 1) / total_task_count;
                std::lock_guard lk(reporter_mutex);
                film_grid.merge_into(
                    image_buffer.value, image_buffer.weight,
                    image_buffer.albedo, image_buffer.normal, image_buffer.denoise);

                reporter.progress(percent, get_img);
            }
            else
            {
                AGZ_UNACCESSED(worker_count);
                AGZ_UNACCESSED(particle_images_lock);
                AGZ_UNACCESSED(particle_output_film);

                total_particle_count += task_particle_count;

                film_grid.merge_into(
                    image_buffer.value, image_buffer.weight,
                    image_buffer.albedo, image_buffer.normal, image_buffer.denoise);

                std::lock_guard lk(reporter_mutex);
                const real percent = real(100) * (task_id + 1) / total_task_count;
                reporter.progress(percent, {});
            }
        }
    };

    // create working threads

    Arena sampler_arena;

    std::vector<std::thread> threads;
    std::vector<Image2D<Spectrum>> particle_images;

    threads.reserve(worker_count);
    particle_images.resize(worker_count, Image2D<Spectrum>(height, width));

    auto sampler_prototype = newRC<NativeSampler>(42, false);

    for(int i = 0; i < worker_count; ++i)
    {
        Sampler *sampler = sampler_prototype->clone(i, sampler_arena);
        threads.emplace_back(render_func, sampler, &particle_images[i], i);
    }

    for(auto &t : threads)
        t.join();

    // construct forward tracing result

    const Image2D<real> image_buffer_ratio = image_buffer.weight.map([](real w)
    {
        return w > 0 ? 1 / w : real(1);
    });

    RenderTarget render_target;
    render_target.image   = image_buffer_ratio * image_buffer.value;
    render_target.albedo  = image_buffer_ratio * image_buffer.albedo;
    render_target.normal  = image_buffer_ratio * image_buffer.normal;
    render_target.denoise = image_buffer_ratio * image_buffer.denoise;

    // merge particle images

    const real particle_scale = static_cast<real>(width * height)
                              / (std::max<uint64_t>)(
                                    1, total_particle_count.load());
    for(auto &pi : particle_images)
        render_target.image = render_target.image + particle_scale * pi;

    reporter.end_stage();
    reporter.end();

    return render_target;
}

int BDPTRenderer::render_grid(
    const Scene &scene, Sampler &sampler, FilmGrid &film_grid,
    ParticleFilm &particle_film, const Vec2i &full_res)
{
    if(scene.lights().empty())
        return 0;

    const Rect2i sample_pixels = film_grid.sample_pixels();
    
    Arena arena;
    std::vector<render::bdpt::BDPTVertex> cam_subpath_space(
        params_.cam_max_vtx_cnt);
    std::vector<render::bdpt::BDPTVertex> lht_subpath_space(
        params_.lht_max_vtx_cnt);

    GridParams grid_params = {
        scene, sampler, film_grid, particle_film,
        { real(full_res.x), real(full_res.y) },
        cam_subpath_space.data(), lht_subpath_space.data()
    };

    int particle_count = 0;
    for(int py = sample_pixels.low.y; py <= sample_pixels.high.y; ++py)
    {
        for(int px = sample_pixels.low.x; px <= sample_pixels.high.x; ++px)
        {
            for(int i = 0; i < params_.spp; ++i)
            {
                ++particle_count;
                eval_sample(px, py, grid_params, arena);

                if(arena.used_bytes() > 4 * 1024 * 1024)
                    arena.release();

                if(stop_rendering_)
                    return particle_count;
            }
        }
    }

    return particle_count;
}

void BDPTRenderer::eval_sample(
    int px, int py, const GridParams &grid_params, Arena &arena)
{
    const auto pixel = trace_bdpt(
        algo_params_, grid_params.scene,
        px, py, grid_params.full_res,
        grid_params.sampler, arena,
        grid_params.cam_subpath_space,
        grid_params.lht_subpath_space,
        &grid_params.particle_film);

    if(pixel)
    {
        grid_params.film_grid.apply(
            pixel->px, pixel->py,
            pixel->pixel.value, 1,
            pixel->pixel.albedo,
            pixel->pixel.normal,
            pixel->pixel.denoise);
    }
}

RC<Renderer> create_bdpt_renderer(const BDPTRendererParams &params)
{
    return newRC<BDPTRenderer>(params);
}

AGZ_TRACER_END
