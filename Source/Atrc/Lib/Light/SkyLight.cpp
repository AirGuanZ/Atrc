#include <Atrc/Lib/Core/Scene.h>
#include <Atrc/Lib/Light/SkyLight.h>

namespace Atrc
{

SkyLight::SkyLight(const Spectrum &topAndBottom)
    : SkyLight(topAndBottom, topAndBottom)
{

}

SkyLight::SkyLight(const Spectrum &top, const Spectrum &bottom)
    : top_(top), bottom_(bottom), worldRadius_(0)
{

}

void SkyLight::PreprocessScene(const Scene &scene)
{
    AGZ_ASSERT(!worldRadius_);
    auto wb = scene.GetWorldBound();
    worldCentre_ = (wb.high + wb.low) / 2;
    worldRadius_ = Real(1.01) * (wb.high - worldCentre_).Length();
}

Light::SampleWiResult SkyLight::SampleWi(
    const Intersection &inct, const ShadingPoint &shd, const Vec2 &sample) const noexcept
{
    auto [sam, pdf] = AGZ::Math::DistributionTransform::
        ZWeightedOnUnitHemisphere<Real>::Transform(sample);
    sam = shd.coordSys.Local2World(sam);

    SampleWiResult ret;
    ret.pos      = inct.pos + 2 * sam * worldRadius_;
    ret.wi       = sam;
    ret.radiance = NonAreaLe(Ray(ret.pos, sam, EPS));
    ret.pdf      = pdf;
    ret.isDelta  = false;
    return ret;
}

Real SkyLight::SampleWiPDF(
    const Vec3 &pos, const Intersection &inct, const ShadingPoint &shd, bool onLight) const noexcept
{
    AGZ_ASSERT(!onLight);
    Vec3 lwi = shd.coordSys.World2Local(pos - inct.pos).Normalize();
    return AGZ::Math::DistributionTransform::
        ZWeightedOnUnitHemisphere<Real>::PDF(lwi);
}

Spectrum SkyLight::AreaLe([[maybe_unused]] const Intersection &inct) const noexcept
{
    return Spectrum();
}

Spectrum SkyLight::NonAreaLe(const Ray &r) const noexcept
{
    float topWeight = 0.5f * float(r.d.Normalize().z) + 0.5f;
    return topWeight * top_ + (1 - topWeight) * bottom_;
}

} // namespace Atrc
