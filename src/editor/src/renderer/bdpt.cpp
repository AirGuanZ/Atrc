#include <agz/editor/renderer/bdpt.h>

AGZ_EDITOR_BEGIN

BDPTRenderer::BDPTRenderer(
    const Params &params, int width, int height, std::shared_ptr<const tracer::Scene> scene)
    : ParticleRenderer(
        params.worker_count, params.task_grid_size, 3, width, height,
        params.enable_preview, 128, 32, scene)
{
    preview_params_.min_depth = (std::max)(1, params.max_cam_depth - 3);
    preview_params_.max_depth = params.max_cam_depth;
    preview_params_.cont_prob = real(0.9);
    preview_params_.direct_illum_sample_count = 1;

    bdpt_params_.max_cam_vtx_cnt = params.max_cam_depth;
    bdpt_params_.max_lht_vtx_cnt = params.max_lht_depth;
    bdpt_params_.use_mis         = params.use_mis;
}

BDPTRenderer::~BDPTRenderer()
{
    stop_rendering();
}

Spectrum BDPTRenderer::fast_render_pixel(
    const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler, tracer::Arena &arena)
{
    return trace_std(preview_params_, scene, ray, sampler, arena).value;
}

Spectrum BDPTRenderer::render_pixel(
    const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler,
    tracer::Arena &arena, tracer::FilmFilterApplier::FilmGridView<Spectrum> &particle_film, uint64_t *particle_count)
{
    return Spectrum(0);
}

uint64_t BDPTRenderer::exec_render_task(
    Framebuffer::Task &task, tracer::Sampler &sampler,
    tracer::FilmFilterApplier::FilmGridView<Spectrum> &particle_film)
{
    using namespace tracer;

    uint64_t ret = 0;

    Arena arena;
    std::vector<render::BDPTVertex> cam_subpath_space(bdpt_params_.max_cam_vtx_cnt);
    std::vector<render::BDPTVertex> lht_subpath_space(bdpt_params_.max_lht_vtx_cnt);

    const Rect2i sam_bound = task.pixel_range;
    
    task.grid->value.clear(Spectrum());
    task.grid->weight.clear(0);

    for(int py = sam_bound.low.y; py <= sam_bound.high.y; ++py)
    {
        for(int px = sam_bound.low.x; px <= sam_bound.high.x; ++px)
        {
            for(int s = 0; s < task.spp; ++s)
            {
                if(stop_rendering_)
                    return ret;

                auto opt_pixel = trace_bdpt(
                    bdpt_params_, *scene_, px, py, task.full_res, sampler, arena,
                    cam_subpath_space.data(), lht_subpath_space.data(), &particle_film);
                ++ret;

                const int lx = px - sam_bound.low.x;
                const int ly = py - sam_bound.low.y;

                if(opt_pixel)
                {
                    task.grid->value(ly, lx)  += opt_pixel->pixel.value;
                    task.grid->weight(ly, lx) += 1;
                }
                else
                {
                    task.grid->value(ly, lx)  += Spectrum(real(0.001));
                    task.grid->weight(ly, lx) += real(0.001);
                }

                if(arena.used_bytes() > 4 * 1024 * 1024)
                    arena.release();
            }
        }
    }

    return ret;
}

AGZ_EDITOR_END
