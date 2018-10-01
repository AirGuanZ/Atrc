#include <Atrc/Entity/AmbientSphere.h>
#include <Atrc/Material/Ambient.h>

AGZ_NS_BEG(Atrc)

AmbientSphere::AmbientSphere(Real radius, const Transform &local2World, const Spectrum &color)
    : Sphere(radius, local2World), ambientColor_(color)
{

}

RC<BxDF> AmbientSphere::GetBxDF(const Intersection &inct) const
{
    return NewRC<AmbientBRDF>(ambientColor_);
}

AGZ_NS_END(Atrc)
