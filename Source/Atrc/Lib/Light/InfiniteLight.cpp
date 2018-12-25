#include <Atrc/Lib/Core/Scene.h>
#include <Atrc/Lib/Light/InfiniteLight.h>

namespace Atrc
{

InfiniteLight::InfiniteLight(const Transform &local2World) noexcept
    : worldRadius_(0), local2World_(local2World)
{

}

void InfiniteLight::PreprocessScene(const Scene &scene)
{
    auto &wb = scene.GetWorldBound();
    worldCentre_ = (wb.high + wb.low) / 2;
    worldRadius_ = Real(1.01) * (wb.high - worldCentre_).Length();
}

Light::SampleWiResult InfiniteLight::SampleWi(
    const Intersection &inct, const ShadingPoint &shd, const Vec3 &sample) const noexcept
{
    auto [sam, pdf] = AGZ::Math::DistributionTransform::
        ZWeightedOnUnitHemisphere<Real>::Transform(sample.xy());
    sam = shd.coordSys.Local2World(sam);

    SampleWiResult ret;
    ret.pos      = inct.pos + 2 * sam * worldRadius_;
    ret.wi       = sam;
    ret.radiance = NonAreaLe(Ray(ret.pos, sam, EPS));
    ret.pdf      = pdf;
    ret.isDelta  = false;
    ret.isInf    = true;

    if(!ret.pdf)
        ret.radiance = Spectrum();
        
    return ret;
}

Real InfiniteLight::SampleWiAreaPDF(
    [[maybe_unused]] const Vec3 &pos, [[maybe_unused]] const Vec3 &nor,
    [[maybe_unused]] const Intersection &inct, [[maybe_unused]] const ShadingPoint &shd) const noexcept
{
    return 0;
}

Real InfiniteLight::SampleWiNonAreaPDF(
    const Vec3 &wi,
    [[maybe_unused]] const Intersection &inct, [[maybe_unused]] const ShadingPoint &shd) const noexcept
{
    Vec3 lwi = shd.coordSys.World2Local(wi).Normalize();
    return AGZ::Math::DistributionTransform::
        ZWeightedOnUnitHemisphere<Real>::PDF(lwi);
}

Light::SampleWiResult InfiniteLight::SampleWi(const Vec3 &pos, const Vec3 &sample) const noexcept
{
    auto[sam, pdf] = AGZ::Math::DistributionTransform::
        UniformOnUnitSphere<Real>::Transform(sample.xy());

    SampleWiResult ret;
    ret.pos = pos + 2 * sam * worldRadius_;
    ret.wi = sam;
    ret.radiance = NonAreaLe(Ray(ret.pos, sam, EPS));
    ret.pdf = pdf;
    ret.isDelta = false;
    ret.isInf = true;

    return ret;
}

Real InfiniteLight::SampleWiAreaPDF(
    [[maybe_unused]] const Vec3 &pos, [[maybe_unused]] const Vec3 &nor,
    [[maybe_unused]] const Vec3 &medPos) const noexcept
{
    return 0;
}

Real InfiniteLight::SampleWiNonAreaPDF([[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &medPos) const noexcept
{
    return AGZ::Math::DistributionTransform::UniformOnUnitSphere<Real>::PDF();
}

Spectrum InfiniteLight::AreaLe([[maybe_unused]] const Intersection &inct) const noexcept
{
    return Spectrum();
}

} // namespace Atrc
