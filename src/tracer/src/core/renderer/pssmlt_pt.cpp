#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/renderer_interactor.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/create/renderer.h>
#include <agz/tracer/render/path_tracing.h>
#include <agz/tracer/render/pssmlt.h>
#include <agz/tracer/utility/parallel_grid.h>
#include <agz-utils/thread.h>

AGZ_TRACER_BEGIN

namespace
{
    struct AtomicSpectrum
    {
        std::atomic<real> channels[SPECTRUM_COMPONENT_COUNT] = { 0 };

        AtomicSpectrum() = default;

        AtomicSpectrum(const AtomicSpectrum &s) noexcept
        {
            for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
                channels[i] = s.channels[i].load();
        }

        void add(const FSpectrum &rhs) noexcept
        {
            for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
                math::atomic_add(channels[i], rhs[i]);
        }

        Spectrum to_spectrum() const noexcept
        {
            Spectrum ret;
            for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
                ret[i] = channels[i].load();
            return ret;
        }
    };
}

class PSSMLTPTRenderer : public Renderer
{
    PSSMLTPTRendererParams params_;

    render::Pixel (*trace_func_)(
        const render::TraceParams&, const Scene&,
        const Ray&, Sampler&, Arena&);

    render::TraceParams trace_params_;

    FSpectrum eval_path(
        const Scene &scene, const Vec2 &film_coord,
        Arena &arena, render::pssmlt::PSSMLTSampler &sampler) const;

public:

    explicit PSSMLTPTRenderer(const PSSMLTPTRendererParams &params);

    RenderTarget render(
        FilmFilterApplier filter, Scene &scene,
        RendererInteractor &reporter) override;
};

RC<Renderer> create_pssmlt_pt_renderer(const PSSMLTPTRendererParams &params)
{
    return newRC<PSSMLTPTRenderer>(params);
}

FSpectrum PSSMLTPTRenderer::eval_path(
    const Scene &scene, const Vec2 &film_coord,
    Arena &arena, render::pssmlt::PSSMLTSampler &sampler) const
{
    const CameraSampleWeResult cam_sam =
        scene.get_camera()->sample_we(film_coord, sampler.sample2());
    const Ray ray(cam_sam.pos_on_cam, cam_sam.pos_to_out);

    const FSpectrum radiance = trace_func_(
        trace_params_, scene, ray, sampler, arena).value;

    return cam_sam.throughput * radiance;
}

PSSMLTPTRenderer::PSSMLTPTRenderer(const PSSMLTPTRendererParams &params)
    : params_(params),
      trace_func_(params.use_mis ? &render::trace_std
                                 : &render::trace_nomis)
{
    trace_params_.min_depth = params.min_depth;
    trace_params_.max_depth = params.max_depth;
    trace_params_.cont_prob = params.cont_prob;
    trace_params_.direct_illum_sample_count = 1;
}

RenderTarget PSSMLTPTRenderer::render(
    FilmFilterApplier filter, Scene &scene,
    RendererInteractor &reporter)
{
    const int thread_count = thread::actual_worker_count(params_.worker_count);

    std::mutex reporter_mutex;
    reporter.begin();

    // per thread resources

    std::vector<Arena> perthread_arenas(thread_count);

    // prepare startup weights

    std::vector<real> startup_weights(params_.startup_sample_count, real(0));
    const int startup_task_size = math::clamp(
        params_.startup_sample_count / 128, 1, 4096);

    int finished_startup_sample = 0;

    reporter.message("compute startup samples");
    reporter.new_stage();

    parallel_for_1d_grid(
        thread_count, params_.startup_sample_count, startup_task_size,
        [&](int thread_index, int beg, int end)
    {
        Arena &arena = perthread_arenas[thread_index];

        for(int i = beg; i < end; ++i)
        {
            if(stop_rendering_)
                return false;

            const NativeSampler native_sampler(i, false);
            render::pssmlt::PSSMLTSampler sampler(
                params_.sigma, params_.large_step_prob, native_sampler);

            const Sample2 film_sample = sampler.sample2();
            const Vec2 film_coord = { film_sample.u, film_sample.v };

            startup_weights[i] = eval_path(
                scene, film_coord, arena, sampler).lum();

            if(arena.used_bytes() > 4 * 1024 * 1024)
                arena.release();
        }

        {
            std::lock_guard lk(reporter_mutex);
            finished_startup_sample += end - beg;

            const real percent = real(100) * finished_startup_sample
                                           / params_.startup_sample_count;
            reporter.progress(percent, {});
        }

        return !stop_rendering_;
    });
    reporter.end_stage();

    // construct startup path sampler

    reporter.message("construct startup path sampler");

    math::distribution::alias_sampler_t<real, int> startup_path_sampler(
        startup_weights.data(), params_.startup_sample_count);

    // estimate b

    reporter.message("estimate b");

    real b_sum = 0;
    for(auto w : startup_weights)
        b_sum += w;
    const real b = b_sum / startup_weights.size();

    // film

    Image2D<AtomicSpectrum> film(filter.height(), filter.width());

    const Rect2i pixel_range = {
        { 0, 0 },
        { filter.width() - 1, filter.height() - 1 }
    };

    const real filter_radius = filter.radius();

    // perthread native samplers

    std::vector<NativeSampler> perthread_native_sampler;
    for(int i = 0; i < thread_count; ++i)
        perthread_native_sampler.push_back(NativeSampler(i, false));

    // how to run a markov chain

    auto run_markov_chain = [&](int thread_index, uint64_t mut_count)
    {
        Arena &local_arena = perthread_arenas[thread_index];

        // sample startup seed

        auto &native_sampler = perthread_native_sampler[thread_index];

        // initialize mlt sampler

        const int mlt_sampler_seed = startup_path_sampler.sample(
            native_sampler.sample1().u);
        render::pssmlt::PSSMLTSampler mlt_sampler(
            params_.sigma, params_.large_step_prob,
            NativeSampler(mlt_sampler_seed, false));

        // first sample

        const Sample2 init_film_sam = mlt_sampler.sample2();
        const Vec2 init_film_coord = { init_film_sam.u, init_film_sam.v };

        Vec2 current_pixel_coord = {
            init_film_coord.x * filter.width(),
            init_film_coord.y * filter.height()
        };

        FSpectrum current_spectrum = eval_path(
            scene, init_film_coord, local_arena, mlt_sampler);

        // do mutations

        for(uint64_t mut_idx = 0; mut_idx < mut_count; ++mut_idx)
        {
            if(stop_rendering_)
                return false;

            mlt_sampler.new_iteration();

            // eval proposed sample

            const Sample2 proposed_film_sam = mlt_sampler.sample2();

            const Vec2 proposed_film_coord  = {
                proposed_film_sam.u, proposed_film_sam.v
            };

            Vec2 proposed_pixel_coord = {
                proposed_film_coord.x * filter.width(),
                proposed_film_coord.y * filter.height()
            };

            FSpectrum proposed_spectrum = eval_path(
                scene, proposed_film_coord, local_arena, mlt_sampler);

            // compute accept prob

            const real accept_prob = (std::min)(
                real(1), proposed_spectrum.lum() / current_spectrum.lum());

            // accumulate to film

            if(accept_prob > 0)
            {
                const FSpectrum proposed_add =
                    proposed_spectrum * accept_prob / proposed_spectrum.lum();

                if(proposed_add.is_finite())
                {
                    apply_image_filter(
                        pixel_range, filter_radius, proposed_pixel_coord,
                        [&](int px, int py, real x_rel, real y_rel)
                    {
                        const real w = filter.eval_filter(x_rel, y_rel);
                        film(py, px).add(w * proposed_add);
                    });
                }
            }

            const FSpectrum current_add =
                current_spectrum * (1 - accept_prob) / current_spectrum.lum();

            if(current_add.is_finite())
            {
                apply_image_filter(
                    pixel_range, filter_radius, current_pixel_coord,
                    [&](int px, int py, real x_rel, real y_rel)
                {
                    const real w = filter.eval_filter(x_rel, y_rel);
                    film(py, px).add(w * current_add);
                });
            }

            // accept/reject

            if(native_sampler.sample1().u < accept_prob)
            {
                current_spectrum    = proposed_spectrum;
                current_pixel_coord = proposed_pixel_coord;

                mlt_sampler.accept();
            }
            else
                mlt_sampler.reject();

            if(local_arena.used_bytes() > 4 * 1024 * 1024)
                local_arena.release();
        }

        return true;
    };

    // run markov chains

    reporter.message("run markov chains");
    reporter.new_stage();

    const uint64_t total_mut_cnt =
        uint64_t(params_.mut_per_pixel) *
        uint64_t(filter.width()) *
        uint64_t(filter.height());

    if(reporter.need_image_preview())
    {
        const int chain_report_interval = math::clamp(
            params_.chain_count / 32, thread_count, 1000);

        thread::thread_group_t thread_group;

        uint64_t finished_mut_cnt = 0;

        for(int chain_idx = 0; chain_idx < params_.chain_count;
            chain_idx += chain_report_interval)
        {
            if(stop_rendering_)
                break;

            const int chain_end = (std::min)(chain_idx + chain_report_interval,
                                             params_.chain_count);
            const int chain_cnt = chain_end - chain_idx;

            parallel_for_1d_grid(thread_count, chain_cnt, 1, thread_group,
                [&](int thread_index, int beg, int end)
            {
                assert(beg + 1 == end);

                beg += chain_idx;

                const uint64_t mut_cnt =
                    (std::min)(
                        total_mut_cnt,
                        uint64_t(beg + 1) * total_mut_cnt
                        / uint64_t(params_.chain_count))
                    - uint64_t(beg) * total_mut_cnt
                    / uint64_t(params_.chain_count);

                if(!run_markov_chain(thread_index, mut_cnt))
                    return false;

                {
                    std::lock_guard lk(reporter_mutex);
                    finished_mut_cnt += mut_cnt;

                    const real percent = real(100) * finished_mut_cnt
                                                   / total_mut_cnt;
                    reporter.progress(percent, {});
                }

                return !stop_rendering_;
            });

            auto get_img = [&]
            {
                const real scale = b / params_.mut_per_pixel
                                 * total_mut_cnt / finished_mut_cnt;

                return film.map([scale](const AtomicSpectrum &s)
                {
                    return scale * s.to_spectrum();
                });
            };
            
            const real percent = real(100) * finished_mut_cnt
                               / total_mut_cnt;
            reporter.progress(percent, get_img);
        }
    }
    else
    {
        int finished_chain_cnt = 0;

        parallel_for_1d_grid(
            thread_count, params_.chain_count, 1,
            [&](int thread_index, int beg, int end)
        {
            assert(beg + 1 == end);

            const uint64_t mut_cnt =
                (std::min)(
                    total_mut_cnt,
                    uint64_t(beg + 1) * total_mut_cnt
                                      / uint64_t(params_.chain_count))
              - uint64_t(beg) * total_mut_cnt
                              / uint64_t(params_.chain_count);

            if(!run_markov_chain(thread_index, mut_cnt))
                return false;

            {
                std::lock_guard lk(reporter_mutex);
                finished_chain_cnt++;

                const real percent = real(100) * finished_chain_cnt
                                               / params_.chain_count;
                reporter.progress(percent, {});
            }

            return !stop_rendering_;
        });
    }

    reporter.end_stage();
    reporter.end();

    // compute final image

    const real scale = b / params_.mut_per_pixel;

    RenderTarget ret;
    ret.image = film.map([scale](const AtomicSpectrum &s)
    {
        return scale * s.to_spectrum();
    });

    return ret;
}

AGZ_TRACER_END
