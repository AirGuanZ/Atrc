#pragma once

#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/render_target.h>
#include <agz/tracer/render/path_tracing.h>

AGZ_TRACER_BEGIN

class PerPixelRenderer : public Renderer
{
    using ImageBuffer = ImageBufferTemplate<true, true, true, true, true>;

    // image value, weight, albedo, normal, denoise
    using Grid = FilmFilterApplier::FilmGrid<
        Spectrum, real, Spectrum, Vec3, real>;

    void render_grid(
        const Scene &scene, Sampler &sampler,
        Grid &grid, const Vec2i &full_res, int spp) const;

    template<bool REPORTER_WITH_PREVIEW>
    RenderTarget render_impl(
        FilmFilterApplier filter, Scene &scene, RendererInteractor &reporter);

    int worker_count_;
    int task_grid_size_;

    int spp_;

protected:

    using Pixel = render::Pixel;

    virtual Pixel eval_pixel(
        const Scene &scene, const Ray &ray,
        Sampler &sampler, Arena &arena) const = 0;

public:

    PerPixelRenderer(int worker_count, int task_grid_size, int spp);

    RenderTarget render(
        FilmFilterApplier filter, Scene &scene,
        RendererInteractor &reporter) override;
};

AGZ_TRACER_END
