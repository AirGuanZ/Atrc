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
        return true;
    }
    return false;
}

AABB GeometricDiffuseLight::GetWorldBound() const
{
    return geometry_->GetWorldBound();
}

const Material *GeometricDiffuseLight::GetMaterial([[maybe_unused]] const Intersection &inct) const noexcept
{
    return &STATIC_IDEAL_BLACK;
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

Light::SampleWiResult GeometricDiffuseLight::SampleWi(const Intersection &inct, [[maybe_unused]] const ShadingPoint &shd, const Vec3 &sample) const noexcept
{
    Geometry::SampleResult gRet = geometry_->Sample(inct.pos, sample);

    SampleWiResult ret;
    ret.pos      = gRet.pos;
    ret.wi       = gRet.pos - inct.pos;
    ret.radiance = radiance_;
    ret.pdf      = gRet.pdf * (ret.pos - inct.pos).LengthSquare() / Cos(gRet.nor, -ret.wi);
    ret.isDelta  = false;

    return ret;
}

Real GeometricDiffuseLight::SampleWiAreaPDF(
    const Vec3 &pos, const Vec3 &nor, const Intersection &inct, [[maybe_unused]] const ShadingPoint &shd) const noexcept
{
    return geometry_->SamplePDF(pos, inct.pos) * (pos - inct.pos).LengthSquare() / Cos(nor, inct.pos - pos);
}

Real GeometricDiffuseLight::SampleWiNonAreaPDF(
    [[maybe_unused]] const Vec3 &wi,
    [[maybe_unused]] const Intersection &inct, [[maybe_unused]] const ShadingPoint &shd) const noexcept
{
    return 0;
}

/*Real GeometricDiffuseLight::SampleWiPDF(
    const Vec3 &pos, const Vec3 &nor, const Intersection &inct,
    [[maybe_unused]] const ShadingPoint &shd, [[maybe_unused]] bool onLight) const noexcept
{
    return onLight ? geometry_->SamplePDF(pos, inct.pos) * (pos - inct.pos).LengthSquare() / Cos(nor, inct.pos - pos) : Real(0);
}*/

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
