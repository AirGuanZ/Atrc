#pragma once

#include "ObjectManager/EntityCreator.h"
#include "ObjectManager/GeometryManager.h"
#include "ObjectManager/IntegratorManager.h"
#include "ObjectManager/LightManager.h"
#include "ObjectManager/MaterialManager.h"
#include "ObjectManager/MediumManager.h"
#include "ObjectManager/PostProcessorManager.h"
#include "ObjectManager/RendererManager.h"
#include "ParamParser/ParamParser.h"

inline void InitializeObjectManagers()
{
    using namespace ObjMgr;

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
        CubeEnvironmentLightCreator::GetInstancePtr(),
        DirectionalLightCreator    ::GetInstancePtr(),
        SkyLightCreator            ::GetInstancePtr(),
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
