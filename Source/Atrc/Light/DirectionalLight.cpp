#include <Atrc/Entity/Entity.h>
#include <Atrc/Light/DirectionalLight.h>
#include "Atrc/Entity/GeometryTemplate/Sphere.h"

AGZ_NS_BEG(Atrc)

DirectionalLight::DirectionalLight(const Spectrum &radiance, const Vec3r &dir, Radr coverRad)
    : radiance_(radiance), dir_(dir.Normalize()), coverRad_(coverRad), worldRadius_(0.0)
{

}

void DirectionalLight::PreprocessScene(const Scene &scene)
{
    world_ = AABB();
    for(auto ent : scene.entities)
        world_ = world_ | ent->GetBoundingBox();

    Vec3r d = world_.high - world_.low;
    if(d.x <= 0.0 || d.y <= 0.0 || d.z <= 0.0)
        worldRadius_ = 0.0;
    else
        worldRadius_ = Sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
}

Option<LightSample> DirectionalLight::SampleTo(const Intersection &inct) const
{
    LightSample ret;

    ret.pos      = inct.pos - 2.0 * worldRadius_ * dir_;
    Real dis = (ret.pos - inct.pos).Length();
    ret.pos += CommonSampler::Uniform_InUnitSphere::Sample().sample
             * dis * Tan(coverRad_ / 2.0);

    ret.nor      = dir_;
    ret.radiance = radiance_ * (ret.pos - inct.pos).LengthSquare();
    ret.pdf      = 1.0;

    return ret;
}

Spectrum DirectionalLight::Le(const Intersection &inct) const
{
    throw UnreachableCodeException("DirectionalLight::Le unreachable");
}

AGZ_NS_END(Atrc)
