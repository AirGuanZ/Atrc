#include <Atrc/Entity/MetalSphere.h>
#include <Atrc/Material/Metal.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

MetalSphere::MetalSphere(Real radius, const Transform &local2World, const Spectrum &color, Real roughness)
    : Sphere(radius, local2World), reflectedColor_(color), roughness_(roughness)
{
    
}

RC<BxDF> MetalSphere::GetBxDF(const Intersection &inct) const
{
    return NewRC<MetalBxDF>(inct, reflectedColor_, roughness_);
}

AGZ_NS_END(Atrc)
