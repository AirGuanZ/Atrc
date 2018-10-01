#include <Atrc/Entity/DiffuseSphere.h>
#include <Atrc/Material/Diffuse.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

DiffuseSphere::DiffuseSphere(Real radius, const Transform &local2World, const Spectrum &color)
    : Sphere(radius, local2World), diffuseColor_(color)
{

}

RC<BxDF> DiffuseSphere::GetBxDF(const Intersection &inct) const
{
    return NewRC<DiffuseBxDF>(inct, diffuseColor_);
}

AGZ_NS_END(Atrc)
