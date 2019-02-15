#include <Atrc/Lib/Core/Geometry.h>
#include <Atrc/Lib/Entity/GeometricDiffuseLight.h>
#include <Atrc/Lib/Material/IdealBlack.h>

namespace Atrc
{
    
GeometricDiffuseLight::GeometricDiffuseLight(const Geometry *geometry, const Spectrum &radiance) noexcept
    : geometry_(geometry), radiance_(radiance)
{
    
}

bool GeometricDiffuseLight::HasIntersection(const Ray &r) const noexcept
{
    return geometry_->HasIntersection(r);
}

bool GeometricDiffuseLight::FindIntersection(const Ray &r, Intersection *inct) const noexcept
{
    if(geometry_->FindIntersection(r, inct))
    {
        inct->entity = this;
        inct->material = &STATIC_IDEAL_BLACK;
        return true;
    }
    return false;
}

AABB GeometricDiffuseLight::GetWorldBound() const
{
    return geometry_->GetWorldBound();
}

const Light *GeometricDiffuseLight::AsLight() const noexcept
{
    return this;
}

Light *GeometricDiffuseLight::AsLight() noexcept
{
    return this;
}

void GeometricDiffuseLight::PreprocessScene([[maybe_unused]] const Scene &scene)
{
    // do nothing
}

Light::SampleWiResult GeometricDiffuseLight::SampleWi(const Vec3 &pos, const Vec3 &sample) const noexcept
{
    Geometry::SampleResult gRet = geometry_->Sample(pos, sample);

    SampleWiResult ret;
    ret.pos = gRet.pos;
    ret.wi = gRet.pos - pos;
    ret.radiance = radiance_;
    ret.pdf = gRet.pdf * (ret.pos - pos).LengthSquare() / Cos(gRet.nor, -ret.wi);
    ret.isDelta = false;
    ret.isInf = false;

    if(Dot(gRet.nor, ret.wi) >= 0)
        ret.radiance = Spectrum();

    return ret;
}

Real GeometricDiffuseLight::SampleWiAreaPDF(const Vec3 &pos, const Vec3 &nor, const Vec3 &dst) const noexcept
{
    return geometry_->SamplePDF(pos, dst) * (pos - dst).LengthSquare() / Cos(nor, dst - pos);
}

Real GeometricDiffuseLight::SampleWiNonAreaPDF([[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &dst) const noexcept
{
    return 0;
}

Spectrum GeometricDiffuseLight::AreaLe([[maybe_unused]] const Intersection &inct) const noexcept
{
    AGZ_ASSERT(inct.entity == this);
    return radiance_;
}

Spectrum GeometricDiffuseLight::NonAreaLe([[maybe_unused]] const Ray &r) const noexcept
{
    return Spectrum();
}

} // namespace Atrc
