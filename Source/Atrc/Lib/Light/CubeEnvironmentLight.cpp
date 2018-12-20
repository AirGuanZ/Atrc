#include <Utils/Texture.h>

#include <Atrc/Lib/Core/Scene.h>
#include <Atrc/Lib/Light/CubeEnvironmentLight.h>

namespace Atrc
{

CubeEnvironmentLight::CubeEnvironmentLight(const Texture **envTex) noexcept
    : worldRadius_(0)
{
    AGZ_ASSERT(envTex);
    for(int i = 0; i < 6; ++i)
    {
        AGZ_ASSERT(envTex[i]);
        envTex_[i] = envTex[i];
    }
}

void CubeEnvironmentLight::PreprocessScene(const Scene &scene)
{
    auto wb = scene.GetWorldBound();
    worldCentre_ = (wb.high + wb.low) / 2;
    worldRadius_ = Real(1.01) * (wb.high - worldCentre_).Length();
}

Light::SampleWiResult CubeEnvironmentLight::SampleWi(
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
    return ret;
}

Real CubeEnvironmentLight::SampleWiAreaPDF(
    [[maybe_unused]] const Vec3 &pos, [[maybe_unused]] const Vec3 &nor,
    [[maybe_unused]] const Intersection &inct, [[maybe_unused]] const ShadingPoint &shd) const noexcept
{
    return 0;
}

Real CubeEnvironmentLight::SampleWiNonAreaPDF(
    const Vec3 &wi,
    [[maybe_unused]] const Intersection &inct, [[maybe_unused]] const ShadingPoint &shd) const noexcept
{
    Vec3 lwi = shd.coordSys.World2Local(wi).Normalize();
    return AGZ::Math::DistributionTransform::
        ZWeightedOnUnitHemisphere<Real>::PDF(lwi);
}

Spectrum CubeEnvironmentLight::AreaLe([[maybe_unused]] const Intersection &inct) const noexcept
{
    return Spectrum();
}

Spectrum CubeEnvironmentLight::NonAreaLe(const Ray &r) const noexcept
{
    auto [face, uv] = AGZ::CubeMapper<Real>::Map(r.d.Normalize());
    return envTex_[int(face)]->Sample(uv);
}

} // namespace Atrc
