#include <agz/factory/creator/aggregate_creators.h>
#include <agz/factory/creator/camera_creators.h>
#include <agz/factory/creator/entity_creators.h>
#include <agz/factory/creator/envir_light_creators.h>
#include <agz/factory/creator/film_filter_creators.h>
#include <agz/factory/creator/geometry_creators.h>
#include <agz/factory/creator/material_creators.h>
#include <agz/factory/creator/medium_creators.h>
#include <agz/factory/creator/post_processor_creators.h>
#include <agz/factory/creator/renderer_creators.h>
#include <agz/factory/creator/renderer_interactor_creators.h>
#include <agz/factory/creator/scene_creators.h>
#include <agz/factory/creator/texture2d_creators.h>
#include <agz/factory/creator/texture3d_creators.h>

AGZ_TRACER_FACTORY_BEGIN

CreatingContext::CreatingContext()
    : factory_tuple_{
        Factory<Aggregate>            ("aggregate"),
        Factory<Camera>               ("camera"),
        Factory<Entity>               ("entity"),
        Factory<EnvirLight>           ("envir_light"),
        Factory<FilmFilter>           ("film_filter"),
        Factory<Geometry>             ("geometry"),
        Factory<Material>             ("material"),
        Factory<Medium>               ("medium"),
        Factory<PostProcessor>        ("post_processor"),
        Factory<Renderer>             ("renderer"),
        Factory<RendererInteractor>   ("reporter"),
        Factory<Scene>                ("scene"),
        Factory<Texture2D>            ("texture2d"),
        Factory<Texture3D>            ("texture3d")
    }
{
    path_mapper    = nullptr;
    reference_root = nullptr;

    initialize_aggregate_factory              (factory<Aggregate>());
    initialize_camera_factory                 (factory<Camera>());
    initialize_entity_factory                 (factory<Entity>());
    initialize_envir_light_factory            (factory<EnvirLight>());
    initialize_film_filter_factory            (factory<FilmFilter>());
    initialize_geometry_factory               (factory<Geometry>());
    initialize_material_factory               (factory<Material>());
    initialize_medium_factory                 (factory<Medium>());
    initialize_post_processor_factory         (factory<PostProcessor>());
    initialize_renderer_factory               (factory<Renderer>());
    initialize_renderer_interactor_factory    (factory<RendererInteractor>());
    initialize_scene_factory                  (factory<Scene>());
    initialize_texture2d_factory              (factory<Texture2D>());
    initialize_texture3d_factory              (factory<Texture3D>());
}

AGZ_TRACER_FACTORY_END
