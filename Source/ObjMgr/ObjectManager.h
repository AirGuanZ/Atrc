#pragma once

#include "ObjectManager/CameraManager.h"
#include "ObjectManager/EntityCreator.h"
#include "ObjectManager/GeometryManager.h"
#include "ObjectManager/IntegratorManager.h"
#include "ObjectManager/LightManager.h"
#include "ObjectManager/MaterialManager.h"
#include "ObjectManager/MediumManager.h"
#include "ObjectManager/PostProcessorManager.h"
#include "ObjectManager/RendererManager.h"

AGZ_NS_BEG(ObjMgr)

// 注册各类内建的Object Creators
inline void InitializeObjectManagers()
{
    using namespace ObjMgr;

    CameraCreator *CAMERA_CREATORS[] =
    {
        PerspectiveCameraCreator::GetInstancePtr(),
    };
    for(auto c : CAMERA_CREATORS)
        CameraManager::GetInstance().AddCreator(c);

    EntityCreator *ENTITY_CREATORS[] =
    {
        GeometricEntityCreator      ::GetInstancePtr(),
        GeometricDiffuseLightCreator::GetInstancePtr(),
    };
    for(auto c : ENTITY_CREATORS)
        EntityManager::GetInstance().AddCreator(c);

    GeometryCreator *GEOMETRY_CREATORS[] =
    {
        CubeCreator       ::GetInstancePtr(),
        SphereCreator     ::GetInstancePtr(),
        TriangleBVHCreator::GetInstancePtr(),
    };
    for(auto c : GEOMETRY_CREATORS)
        GeometryManager::GetInstance().AddCreator(c);

    LightCreator *LIGHT_CREATORS[] =
    {
        CubeEnvironmentLightCreator  ::GetInstancePtr(),
        DirectionalLightCreator      ::GetInstancePtr(),
        SkyLightCreator              ::GetInstancePtr(),
        SphereEnvironmentLightCreator::GetInstancePtr(),
    };
    for(auto c : LIGHT_CREATORS)
        LightManager::GetInstance().AddCreator(c);

    IntegratorCreator *INTEGRATOR_CREATORS[] =
    {
        AmbientOcclusionIntegratorCreator::GetInstancePtr(),
        PathTracerCreator                ::GetInstancePtr(),
        PureColorIntegratorCreator       ::GetInstancePtr(),
        VolumetricPathTracerCreator      ::GetInstancePtr(),
    };
    for(auto c : INTEGRATOR_CREATORS)
        IntegratorManager::GetInstance().AddCreator(c);

    MaterialCreator *MATERIAL_CREATORS[] =
    {
        BlackMaterialCreator     ::GetInstancePtr(),
        DiffuseMaterialCreator   ::GetInstancePtr(),
        FresnelSpecularCreator   ::GetInstancePtr(),
        IdealMirrorCreator       ::GetInstancePtr(),
        MetalCreator             ::GetInstancePtr(),
        PlasticCreator           ::GetInstancePtr(),
        TextureScalerCreator     ::GetInstancePtr(),
        UncallableMaterialCreator::GetInstancePtr(),
    };
    for(auto c : MATERIAL_CREATORS)
        MaterialManager::GetInstance().AddCreator(c);

    MediumCreator *MEDIUM_CREATORS[] =
    {
        HomongeneousMediumCreator::GetInstancePtr(),
    };
    for(auto c : MEDIUM_CREATORS)
        MediumManager::GetInstance().AddCreator(c);

    PostProcessorStageCreator *POSTPROCESSOR_STAGE_CREATORS[] =
    {
        ACESFilmCreator      ::GetInstancePtr(),
        GammaCorrectorCreator::GetInstancePtr(),
    };
    for(auto c : POSTPROCESSOR_STAGE_CREATORS)
        PostProcessorStageManager::GetInstance().AddCreator(c);

    ProgressReporterCreator *REPORTER_CREATORS[] =
    {
        DefaultProgressReporterCreator::GetInstancePtr(),
    };
    for(auto c : REPORTER_CREATORS)
        ProgressReporterManager::GetInstance().AddCreator(c);

    RendererCreator *RENDERER_CREATORS[] =
    {
        ParallelRendererCreator::GetInstancePtr(),
        SerialRendererCreator  ::GetInstancePtr(),
    };
    for(auto c : RENDERER_CREATORS)
        RendererManager::GetInstance().AddCreator(c);

    SubareaRendererCreator *SUBAREARENDERER_CREATORS[] =
    {
        JitteredSubareaRendererCreator::GetInstancePtr(),
    };
    for(auto c : SUBAREARENDERER_CREATORS)
        SubareaRendererManager::GetInstance().AddCreator(c);
}

// 创建在params内公开定义的元素
inline void InitializePublicObjects(const AGZ::ConfigGroup &params, AGZ::ObjArena<> &arena)
{
    InitializePublicDefinition<Atrc::Geometry>        ("pub_geometry",      params, arena);
    InitializePublicDefinition<Atrc::Material>        ("pub_material",      params, arena);
    InitializePublicDefinition<Atrc::Light>           ("pub_light",         params, arena);
    InitializePublicDefinition<Atrc::Medium>          ("pub_medium",        params, arena);
    InitializePublicDefinition<Atrc::Entity>          ("pub_entity",        params, arena);
    InitializePublicDefinition<Atrc::PostProcessStage>("pub_postprocessor", params, arena);
    InitializePublicDefinition<Atrc::Camera>          ("pub_camera",        params, arena);
    InitializePublicDefinition<Atrc::Integrator>      ("pub_integrator",    params, arena);
    InitializePublicDefinition<Atrc::SubareaRenderer> ("pub_subrenderer",   params, arena);
    InitializePublicDefinition<Atrc::Renderer>        ("pub_renderer",      params, arena);
    InitializePublicDefinition<Atrc::ProgressReporter>("pub_reporter",      params, arena);
}

AGZ_NS_END(ObjMgr)
