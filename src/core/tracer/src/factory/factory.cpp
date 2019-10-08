#include <agz/tracer/factory/creator/aggregate_creators.h>
#include <agz/tracer/factory/creator/camera_creators.h>
#include <agz/tracer/factory/creator/entity_creators.h>
#include <agz/tracer/factory/creator/nonarea_light_creators.h>
#include <agz/tracer/factory/creator/film_creators.h>
#include <agz/tracer/factory/creator/film_filter_creators.h>
#include <agz/tracer/factory/creator/fresnel_creators.h>
#include <agz/tracer/factory/creator/geometry_creators.h>
#include <agz/tracer/factory/creator/material_creators.h>
#include <agz/tracer/factory/creator/medium_creators.h>
#include <agz/tracer/factory/creator/path_tracing_integrator_creators.h>
#include <agz/tracer/factory/creator/post_processor_creators.h>
#include <agz/tracer/factory/creator/renderer_creators.h>
#include <agz/tracer/factory/creator/reporter_creators.h>
#include <agz/tracer/factory/creator/sampler_creators.h>
#include <agz/tracer/factory/creator/scene_creators.h>
#include <agz/tracer/factory/creator/texture_creators.h>

AGZ_TRACER_FACTORY_BEGIN

CreatingContext::CreatingContext()
    : factory_tuple_{
        Factory<Aggregate>            ("aggregate"),
        Factory<Camera>               ("camera"),
        Factory<Entity>               ("entity"),
        Factory<NonareaLight>         ("nonarea_light"),
        Factory<Film>                 ("film"),
        Factory<FilmFilter>           ("film_filter"),
        Factory<Fresnel>              ("fresnel"),
        Factory<Geometry>             ("geometry"),
        Factory<Material>             ("material"),
        Factory<Medium>               ("medium"),
        Factory<PathTracingIntegrator>("integrator"),
        Factory<PostProcessor>        ("post_processor"),
        Factory<Renderer>             ("renderer"),
        Factory<ProgressReporter>     ("reporter"),
        Factory<Sampler>              ("sampler"),
        Factory<Scene>                ("scene"),
        Factory<Texture>              ("texture")
    }
{
    path_mapper = nullptr;
    reference_root = nullptr;

    initialize_aggregate_factory              (factory<Aggregate>());
    initialize_camera_factory                 (factory<Camera>());
    initialize_entity_factory                 (factory<Entity>());
    initialize_nonarea_light_factory          (factory<NonareaLight>());
    initialize_film_factory                   (factory<Film>());
    initialize_film_filter_factory            (factory<FilmFilter>());
    initialize_fresnel_factory                (factory<Fresnel>());
    initialize_geometry_factory               (factory<Geometry>());
    initialize_material_factory               (factory<Material>());
    initialize_medium_factory                 (factory<Medium>());
    initialize_path_tracing_integrator_factory(factory<PathTracingIntegrator>());
    initialize_post_processor_factory         (factory<PostProcessor>());
    initialize_renderer_factory               (factory<Renderer>());
    initialize_reporter_factory               (factory<ProgressReporter>());
    initialize_sampler_factory                (factory<Sampler>());
    initialize_scene_factory                  (factory<Scene>());
    initialize_texture_factory                (factory<Texture>());
}

AGZ_TRACER_FACTORY_END
