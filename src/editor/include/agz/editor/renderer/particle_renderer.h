#pragma once

#include <agz/editor/renderer/renderer.h>

AGZ_EDITOR_BEGIN

class ParticleRenderer : public Renderer
{
public:

    ParticleRenderer(
        int worker_count, int task_grid_size, int init_pixel_size,
        int framebuffer_width, int framebuffer_height,
        bool enable_fast_rendering, int fast_resolution, int fast_task_size,
        RC<const tracer::Scene> scene);

    ~ParticleRenderer();

    Image2D<Spectrum> start() override;

    Image2D<Spectrum> get_image() const override;

protected:

    void stop_rendering();

    virtual Spectrum fast_render_pixel(
        const tracer::Scene &scene, const tracer::Ray &ray,
        tracer::Sampler &sampler, tracer::Arena &arena) = 0;

    virtual Spectrum render_pixel(
        const tracer::Scene &scene, const tracer::Ray &ray,
        tracer::Sampler &sampler, tracer::Arena &arena,
        tracer::FilmFilterApplier::FilmGridView<Spectrum> &particle_film,
        uint64_t *particle_count) = 0;

    virtual uint64_t exec_render_task(
        Framebuffer::Task &task, tracer::Sampler &sampler,
        tracer::FilmFilterApplier::FilmGridView<Spectrum> &particle_film);

private:

    Image2D<Spectrum> do_fast_rendering();

    void exec_fast_render_task(
        Image2D<Spectrum> &target,
        const Vec2i &beg, const Vec2i &end, tracer::Sampler &sampler);

protected:

    int framebuffer_width_;
    int framebuffer_height_;

    RC<const tracer::Scene> scene_;

    std::atomic<bool> stop_rendering_;

private:

    int worker_count_;

    bool enable_fast_rendering_;
    int fast_resolution_;
    int fast_task_grid_size_;

    Framebuffer framebuffer_;

    std::vector<std::thread> threads_;

    std::thread compute_img_thread_;
    Image2D<Spectrum> output_img_;
    mutable std::mutex output_img_mutex_;

    Box<std::mutex[]> particle_film_mutex_;
    std::vector<Image2D<Spectrum>> particle_film_;
    std::atomic<uint64_t> total_particle_count_;

    Box<tracer::FilmFilterApplier> film_filter_;
    tracer::Arena sampler_arena_;

    std::atomic<int> global_particle_film_version_;
};

AGZ_EDITOR_END
