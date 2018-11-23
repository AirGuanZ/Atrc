#include "SceneManager.h"

using namespace AGZ;
using namespace ObjMgr;

namespace
{
    const Atrc::Camera *ParseCamera(const ConfigGroup &params, Atrc::Real aspectRatio, ObjArena<> &arena)
    {
        auto eye = ParamParser::ParseVec3(params["eye"]);
        auto dst = ParamParser::ParseVec3(params["dst"]);
        auto dir = dst - eye;

        auto up = ParamParser::ParseVec3(params["up"]);

        Atrc::Rad rad;
        auto &angle = params["FOVz"].AsArray();
        if(angle.Size() != 2)
            throw SceneInitializationException("Scene: invalid angle form for FOVy");
        if(angle[0].AsValue() == "Rad")
            rad = Atrc::Rad(angle[1].AsValue().Parse<Atrc::Real>());
        else if(angle[0].AsValue() == "Deg")
            rad = Atrc::Deg(angle[1].AsValue().Parse<Atrc::Real>());
        else
            throw SceneInitializationException("Scene: invalid angle form for FOVy");

        return arena.Create<Atrc::PerspectiveCamera>(
            eye, dir, up, rad, aspectRatio);
    }

    template<typename T>
    void InitializePublicDefinition(const Str8 &fieldName, const ConfigGroup &root, ObjArena<> &arena)
    {
        auto grp = root.Find(fieldName);
        if(grp)
            ObjectManager<T>::GetInstance().InitializePublicDefinitions(grp->AsGroup(), arena);
    }
}

void SceneManager::Initialize(const ConfigGroup &params)
{
    if(IsAvailable())
        throw SceneInitializationException("Scene: reinitialized");

    // 摄像机

    auto outputWidth  = params["output.width"].AsValue().Parse<uint32_t>();
    auto outputHeight = params["output.height"].AsValue().Parse<uint32_t>();
    auto aspectRatio = Atrc::Real(outputWidth) / outputHeight;

    auto camera = ParseCamera(params["camera"].AsGroup(), aspectRatio, arena_);

    std::vector<Atrc::Entity*> entities;
    std::vector<Atrc::Light*> lights;

    // 预定义元素

    InitializePublicDefinition<Atrc::Geometry>("pub_geometry", params, arena_);
    InitializePublicDefinition<Atrc::Light>   ("pub_light",    params, arena_);
    InitializePublicDefinition<Atrc::Material>("pub_material", params, arena_);
    InitializePublicDefinition<Atrc::Medium>  ("pub_medium",   params, arena_);
    InitializePublicDefinition<Atrc::Entity>  ("pub_entity",   params, arena_);

    InitializePublicDefinition<Atrc::Integrator>      ("pub_integrator", params, arena_);
    InitializePublicDefinition<Atrc::Renderer>        ("pub_renderer",   params, arena_);
    InitializePublicDefinition<Atrc::ProgressReporter>("pub_reporter", params, arena_);

    // 创建实体

    auto &entArr = params["entities"].AsArray();
    for(size_t i = 0; i < entArr.Size(); ++i)
    {
        auto ent = EntityManager::GetInstance().GetSceneObject(entArr[i], arena_);
        if(!ent)
            throw SceneInitializationException("SceneManager: unknown entity type");
        entities.push_back(ent);
        
        auto light = ent->AsLight();
        if(light)
            lights.push_back(light);
    }

    // 创建光源

    auto &lgtArr = params["lights"].AsArray();
    for(size_t i = 0; i < lgtArr.Size(); ++i)
    {
        auto light = LightManager::GetInstance().GetSceneObject(lgtArr[i], arena_);
        if(!light)
            throw SceneInitializationException("SceneManager: unknown light type");
        lights.push_back(light);
    }

    scene_.camera = camera;

    for(auto ent : entities)
        scene_.entities_.push_back(ent);
    for(auto lgt : lights)
        scene_.lights_.push_back(lgt);

    for(auto light : lights)
        light->PreprocessScene(scene_);
}

bool SceneManager::IsAvailable() const
{
    return scene_.camera != nullptr;
}

const Atrc::Scene &SceneManager::GetScene() const
{
    if(!IsAvailable())
        throw SceneInitializationException("Scene: uninitialized");
    return scene_;
}
