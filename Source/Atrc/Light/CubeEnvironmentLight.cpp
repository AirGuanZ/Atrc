#include <Atrc/Light/CubeEnvironmentLight.h>

AGZ_NS_BEG(Atrc)

CubeEnvironmentLight::CubeEnvironmentLight(const AGZ::Texture2D<Spectrum> **envTex)
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
    AGZ_ASSERT(!worldRadius_);

    auto &wb = scene.GetWorldBound();
    if(wb.IsEmpty())
    {
        worldRadius_ = 1;
        return;
    }

    worldCentre_ = Real(0.5) * (wb.high - wb.low);

    auto hd = wb.high - worldCentre_;
    worldRadius_ = Real(1.01) * hd.Length();
}

LightSampleToResult CubeEnvironmentLight::SampleLi(const SurfacePoint &sp) const
{
    Real u = Rand(), v = Rand();
    auto [sam, pdf] = AGZ::Math::DistributionTransform::
        ZWeightedOnUnitHemisphere<Real>::Transform({ u, v });
    sam = sp.geoLocal.Local2World(sam).Normalize();

    LightSampleToResult ret;
    ret.pos      = sp.pos + 2 * sam * worldRadius_;
    ret.wi       = sam;
    ret.radiance = NonareaLe(Ray(ret.pos, sam, EPS));
    ret.pdf      = pdf;

    return ret;
}

Real CubeEnvironmentLight::SampleLiPDF(const Vec3 &pos, const SurfacePoint &dst, bool posOnLight) const
{
    AGZ_ASSERT(!posOnLight);

    Vec3 lwi = dst.geoLocal.World2Local(pos - dst.pos).Normalize();
    return AGZ::Math::DistributionTransform::
        ZWeightedOnUnitHemisphere<Real>::PDF(lwi);
}

bool CubeEnvironmentLight::IsDeltaPosition() const
{
    return false;
}

bool CubeEnvironmentLight::IsDeltaDirection() const
{
    return false;
}

bool CubeEnvironmentLight::IsDelta() const
{
    return false;
}

Spectrum CubeEnvironmentLight::AreaLe([[maybe_unused]] const SurfacePoint &sp) const
{
    return Spectrum();
}

bool CubeEnvironmentLight::IgnoreFirstMedium() const
{
    return true;
}

Spectrum CubeEnvironmentLight::NonareaLe(const Ray &r) const
{
    auto texCoord = AGZ::CubeMapper<Real>::Map(r.dir);
    return AGZ::LinearSampler::Sample(*envTex_[static_cast<int>(texCoord.face)], Vec2(texCoord.uv.u, 1 - texCoord.uv.v));
}

Spectrum CubeEnvironmentLight::Power() const
{
    // TODO
    return Spectrum(1.0f);
}

AGZ_NS_END(Atrc)
