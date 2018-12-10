#include <Atrc/Lib/Core/Scene.h>

namespace Atrc
{

Scene::Scene(
    const Entity **entities, size_t entityCount,
    const Light **lights,    size_t lightCount,
    const Camera *camera)
    : entities_(entityCount), lights_(lightCount), camera_(camera)
{
    AGZ_ASSERT((!entityCount || entities) && (!lightCount || lights) && camera);

    std::memcpy(entities_.data(), entities, entityCount * sizeof(const Entity*));
    std::memcpy(lights_.data(),   lights,   lightCount  * sizeof(const Light*));

    for(auto ent : GetEntities())
        worldBound_ |= ent->GetWorldBound();
}

const Camera *Scene::GetCamera() const noexcept
{
    return camera_;
}

const std::vector<const Entity*> Scene::GetEntities() const noexcept
{
    return entities_;
}

const std::vector<const Light*> Scene::GetLights() const noexcept
{
    return lights_;
}

const AABB &Scene::GetWorldBound() const noexcept
{
    return worldBound_;
}

Option<Scene::SampleLightResult> Scene::SampleLight() const noexcept
{
    if(lights_.empty())
        return None;
    size_t index = AGZ::Math::Random::Uniform<size_t>(0, lights_.size() - 1);
    return SampleLightResult {
        lights_[index],
        Real(1) / lights_.size()
    };
}

Real Scene::SampleLightPDF([[maybe_unused]] const Light *light) const noexcept
{
    return Real(1) / lights_.size();
}

bool Scene::HasIntersection(const Ray &r) const noexcept
{
    for(auto ent : GetEntities())
    {
        if(ent->HasIntersection(r))
            return true;
    }
    return false;
}

bool Scene::FindIntersection(const Ray &r, Intersection *inct) const noexcept
{
    inct->t = RealT::Infinity();
    Intersection nInct;
    for(auto ent : GetEntities())
    {
        if(ent->FindIntersection(r, &nInct) && nInct.t < inct->t)
            *inct = nInct;
    }
    return !RealT(inct->t).IsInfinity();
}

} // namespace Atrc
