#include <agz/factory/creator/renderer_creators.h>
#include <agz/tracer/create/renderer.h>

AGZ_TRACER_FACTORY_BEGIN

namespace renderer
{

    class AORendererCreator : public Creator<Renderer>
    {
    public:

        std::string name() const override
        {
            return "ao";
        }

        RC<Renderer> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            AORendererParams ao_params;

            ao_params.worker_count    = params.child_int_or("worker_count", 0);
            ao_params.task_grid_size  = params.child_int_or("task_grid_size", 32);
            ao_params.ao_sample_count = params.child_int_or("ao_sample_count", 5);

            ao_params.low_color              = params.child_spectrum_or("low_color", Spectrum(0));
            ao_params.high_color             = params.child_spectrum_or("high_color", Spectrum(1));
            ao_params.max_occlusion_distance = params.child_real_or("max_occlusion_distance", 1);

            ao_params.background_color = params.child_spectrum_or("background_color", Spectrum(0));

            ao_params.spp = params.child_int("spp");

            return create_ao_renderer(ao_params);
        }
    };

    class BDPTRendererCreator : public Creator<Renderer>
    {
    public:

        std::string name() const override
        {
            return "bdpt";
        }

        RC<Renderer> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            BDPTRendererParams bdpt_params;

            bdpt_params.worker_count   = params.child_int_or("worker_count", 0);
            bdpt_params.task_grid_size = params.child_int_or("task_grid_size", 32);

            bdpt_params.cam_max_vtx_cnt = params.child_int_or("camera_max_depth", 10) + 1;
            bdpt_params.lht_max_vtx_cnt = params.child_int_or("light_max_depth", 10) + 1;
            
            bdpt_params.spp = params.child_int("spp");

            return create_bdpt_renderer(bdpt_params);
        }
    };

    class ParticleTracingRendererCreator : public Creator<Renderer>
    {
    public:

        std::string name() const override
        {
            return "particle";
        }

        RC<Renderer> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            ParticleTracingRendererParams renderer_params;

            renderer_params.worker_count = params.child_int_or("worker_count", 0);

            renderer_params.particle_task_count = params.child_int("particle_task_count");
            renderer_params.particles_per_task  = params.child_int("particles_per_task");
            renderer_params.min_depth           = params.child_int_or("min_depth", 5);
            renderer_params.max_depth           = params.child_int_or("max_depth", 10);
            renderer_params.cont_prob           = params.child_real_or("cont_prob", real(0.9));

            renderer_params.forward_task_grid_size = params.child_int_or("forward_task_grid_size", 32);
            renderer_params.forward_spp            = params.child_int("forward_spp");

            return create_particle_tracing_renderer(renderer_params);
        }
    };

    class PathTracingRendererCreator : public Creator<Renderer>
    {
    public:

        std::string name() const override
        {
            return "pt";
        }

        RC<Renderer> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const int worker_count   = params.child_int_or("worker_count", 0);
            const int task_grid_size = params.child_int_or("task_grid_size", 32);
            const int spp            = params.child_int("spp");;

            const int min_depth  = params.child_int_or("min_depth", 5);
            const int max_depth  = params.child_int_or("max_depth", 10);
            const real cont_prob = params.child_real_or("cont_prob", real(0.9));

            const bool use_mis = params.child_int_or("use_mis", 1) != 0;

            PathTracingRendererParams pt_params;
            pt_params.worker_count      = worker_count;
            pt_params.task_grid_size    = task_grid_size;
            pt_params.spp               = spp;
            pt_params.min_depth         = min_depth;
            pt_params.max_depth         = max_depth;
            pt_params.cont_prob         = cont_prob;
            pt_params.use_mis           = use_mis;

            return create_path_tracing_renderer(pt_params);
        }
    };

    class SPPMRendererCreator : public Creator<Renderer>
    {
    public:

        std::string name() const override
        {
            return "sppm";
        }

        std::shared_ptr<Renderer> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            SPPMRendererParams p;

            p.worker_count           = params.child_int_or("worker_count", 0);
            p.forward_task_grid_size = params.child_int_or("task_grid_size", 128);
            p.forward_max_depth      = params.child_int_or("forward_max_depth", 8);

            p.init_radius = params.child_real_or("init_radius", -1);

            p.iteration_count       = params.child_int("iteration_count");
            p.photons_per_iteration = params.child_int("photons_per_iteration");

            p.photon_min_depth = params.child_int_or("photon_min_depth", 5);
            p.photon_max_depth = params.child_int_or("photon_max_depth", 10);
            p.photon_cont_prob = params.child_real_or("photon_cont_prob", real(0.9));

            p.update_alpha = params.child_real_or("alpha", real(2) / 3);

            p.grid_accel_resolution = params.child_int_or("grid_res", 64);

            return create_sppm_renderer(p);
        }
    };

} // namespace renderer

void initialize_renderer_factory(Factory<Renderer> &factory)
{
    factory.add_creator(newBox<renderer::AORendererCreator>());
    factory.add_creator(newBox<renderer::BDPTRendererCreator>());
    factory.add_creator(newBox<renderer::ParticleTracingRendererCreator>());
    factory.add_creator(newBox<renderer::PathTracingRendererCreator>());
    factory.add_creator(newBox<renderer::SPPMRendererCreator>());
}

AGZ_TRACER_FACTORY_END
