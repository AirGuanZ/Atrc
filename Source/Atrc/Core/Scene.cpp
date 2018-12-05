#include <Atrc/Core/Scene.h>

AGZ_NS_BEG(Atrc)

const Camera* Scene::GetCamera() const
{
    AGZ_ASSERT(camera);
    return camera;
}

const Medium* Scene::GetGlobalMedium() const
{
    return globalMedium;
}

const std::vector<const Light*> &Scene::GetLights() const
{
    return lights_;
}

const std::vector<const Entity*> &Scene::GetEntities() const
{
    return entities_;
}

const AABB &Scene::GetWorldBound() const
{
    if(!worldBound_)
    {
        AABB wb;
        for(auto ent : GetEntities())
            wb = wb | ent->WorldBound();
        worldBound_ = wb;
    }
    return *worldBound_;
}

SceneLightSampleResult Scene::SampleLight() const
{
    if(lights_.empty())
        return { nullptr, Real(0) };

    return {
        lights_[AGZ::Math::Random::Uniform<size_t>(0, lights_.size() - 1)],
        Real(1) / lights_.size()
    };
}

Real Scene::SampleLightPDF([[maybe_unused]] const Light *light) const
{
    AGZ_ASSERT(light && lights_.size());
    return Real(1) / lights_.size();
}

bool Scene::HasIntersection(const Ray &r) const
{
    AGZ_ASSERT(r.IsNormalized());
    for(auto ent : entities_)
    {
        if(ent->HasIntersection(r))
            return true;
    }
    return false;
}

bool Scene::FindCloestIntersection(const Ray &r, SurfacePoint *sp) const
{
    sp->t = RealT::Infinity();
    for(auto ent : entities_)
    {
        SurfacePoint newSp;
        if(ent->FindIntersection(r, &newSp) && newSp.t < sp->t)
            *sp = newSp;
    }
    return !RealT(sp->t).IsInfinity();
}

AGZ_NS_END(Atrc)
