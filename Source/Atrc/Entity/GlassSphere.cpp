#include <Atrc/Entity/GlassSphere.h>
#include <Atrc/Material/Glass.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

GlassSphere::GlassSphere(
    Real radius, const Transform &local2World,
    const Spectrum &reflColor, const Spectrum &refrColor, Real refIdx)
    : Sphere(radius, local2World),
      reflectedColor_(reflColor), refractedColor_(refrColor), refIdx_(refIdx)
{
    
}

RC<BxDF> GlassSphere::GetBxDF(const Intersection &inct) const
{
    return NewRC<GlassBxDF>(inct, reflectedColor_, refractedColor_, refIdx_);
}

AGZ_NS_END(Atrc)
