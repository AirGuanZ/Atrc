#include <agz/editor/renderer/bdpt.h>

AGZ_EDITOR_BEGIN

BDPTRenderer::BDPTRenderer(
    const Params &params, int width, int height, RC<const tracer::Scene> scene)
    : ParticleRenderer(
        params.worker_count, params.task_grid_size, 3, width, height,
        params.enable_preview, 128, 32, scene),
      params_(params)
{
    preview_params_.min_depth = (std::max)(1, params.max_cam_depth - 3);
    preview_params_.max_depth = params.max_cam_depth;
    preview_params_.cont_prob = real(0.9);
    preview_params_.direct_illum_sample_count = 1;
}

BDPTRenderer::~BDPTRenderer()
{
    stop_rendering();
}

Spectrum BDPTRenderer::fast_render_pixel(
    const tracer::Scene &scene, const tracer::Ray &ray,
    tracer::Sampler &sampler, tracer::Arena &arena)
{
    return trace_std(preview_params_, scene, ray, sampler, arena).value;
}

Spectrum BDPTRenderer::render_pixel(
    const tracer::Scene &scene, const tracer::Ray &ray,
    tracer::Sampler &sampler, tracer::Arena &arena,
    tracer::FilmFilterApplier::FilmGridView<Spectrum> &particle_film,
    uint64_t *particle_count)
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
    std::vector<render::vol_bdpt::Vertex> cam_subpath_space(
        params_.max_cam_depth);
    std::vector<render::vol_bdpt::Vertex> lht_subpath_space(
        params_.max_lht_depth);

    const Rect2i sam_bound = task.pixel_range;
    
    task.grid->value.clear(Spectrum());
    task.grid->weight.clear(0);

    const Vec2 forward_full_res = task.full_res;
    const Vec2 particle_full_res = {
        real(framebuffer_width_), real(framebuffer_height_)
    };

    render::vol_bdpt::EvalBDPTPathParams eval_params = {
        *scene_, particle_film.get_sample_pixel_bound(),
        particle_full_res, sampler
    };

    for(int py = sam_bound.low.y; py <= sam_bound.high.y; ++py)
    {
        for(int px = sam_bound.low.x; px <= sam_bound.high.x; ++px)
        {
            for(int s = 0; s < task.spp; ++s)
            {
                if(stop_rendering_)
                    return ret;

                // sample camera ray

                const Sample2 &film_sam = sampler.sample2();

                const Vec2 pixel_coord = {
                    px + film_sam.u,
                    py + film_sam.v
                };

                const Vec2 film_coord = {
                    pixel_coord.x / forward_full_res.x,
                    pixel_coord.y / forward_full_res.y
                };

                const auto cam_sam = scene_->get_camera()->sample_we(
                    film_coord, sampler.sample2());
                const Ray cam_ray(cam_sam.pos_on_cam, cam_sam.pos_to_out);

                // build camera/light subpath

                const auto camera_subpath = build_camera_subpath(
                    params_.max_cam_depth, cam_ray, *scene_,
                    sampler, arena, cam_subpath_space.data());

                const auto select_light = scene_->sample_light(sampler.sample1());
                if(!select_light.light)
                    continue;

                const auto light_subpath = build_light_subpath(
                    params_.max_lht_depth, select_light, *scene_,
                    sampler, arena,lht_subpath_space.data());

                // connect subpaths

                const Spectrum radiance = render::vol_bdpt::eval_bdpt_path<true>(
                    eval_params,
                    camera_subpath.vertices, camera_subpath.vertex_count,
                    light_subpath.vertices, light_subpath.vertex_count,
                    select_light, [&](const Vec2 &particle_coord, const Spectrum &rad)
                {
                    if(rad.is_finite())
                    {
                        particle_film.apply(particle_coord.x, particle_coord.y, rad);
                    }
                });

                ++ret;

                const int lx = px - sam_bound.low.x;
                const int ly = py - sam_bound.low.y;

                if(radiance.is_finite())
                {
                    task.grid->value(ly, lx)  += radiance;
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
