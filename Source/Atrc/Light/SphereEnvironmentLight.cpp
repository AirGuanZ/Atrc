#include <Atrc/Light/SphereEnvironmentLight.h>

AGZ_NS_BEG(Atrc)

SphereEnvironmentLight::SphereEnvironmentLight(const Texture *envTex, const Transform &local2World)
    : envTex_(envTex), local2World_(local2World), worldRadius_(Real(0))
{
    AGZ_ASSERT(envTex);
}

void SphereEnvironmentLight::PreprocessScene(const Scene &scene)
{
    AGZ_ASSERT(!worldRadius_);

    auto &wb = scene.GetWorldBound();

    worldCentre_ = Real(0.5) * (wb.high + wb.low);

    auto hd = wb.high - worldCentre_;
    worldRadius_ = Real(1.1) * hd.Length();
}

LightSampleToResult SphereEnvironmentLight::SampleLi(const SurfacePoint &sp) const
{
    Real u = Rand(), v = Rand();
    auto[sam, pdf] = AGZ::Math::DistributionTransform::
        ZWeightedOnUnitHemisphere<Real>::Transform({ u, v });
    sam = sp.geoLocal.Local2World(sam).Normalize();

    LightSampleToResult ret;
    ret.pos = sp.pos + 2 * sam * worldRadius_;
    ret.wi = sam;
    ret.radiance = NonareaLe(Ray(ret.pos, sam, EPS));
    ret.pdf = pdf;

    return ret;
}

Real SphereEnvironmentLight::SampleLiPDF(const Vec3 &pos, const SurfacePoint &dst, [[maybe_unused]] bool posOnLight) const
{
    AGZ_ASSERT(!posOnLight);

    Vec3 lwi = dst.geoLocal.World2Local(pos - dst.pos).Normalize();
    return AGZ::Math::DistributionTransform::
        ZWeightedOnUnitHemisphere<Real>::PDF(lwi);
}

bool SphereEnvironmentLight::IsDeltaPosition() const
{
    return false;
}

bool SphereEnvironmentLight::IsDeltaDirection() const
{
    return false;
}

bool SphereEnvironmentLight::IsDelta() const
{
    return false;
}

Spectrum SphereEnvironmentLight::AreaLe([[maybe_unused]] const SurfacePoint &sp) const
{
    return Spectrum();
}

bool SphereEnvironmentLight::IgnoreFirstMedium() const
{
    return true;
}

Spectrum SphereEnvironmentLight::NonareaLe(const Ray &r) const
{
    AGZ_ASSERT(r.IsNormalized());
    auto texCoord = AGZ::SphereMapper<Real>::Map(local2World_.ApplyInverseToVector(r.dir));
    return envTex_->Sample(texCoord);
}

Spectrum SphereEnvironmentLight::Power() const
{
    // TODO
    return Spectrum(1.0f);
}

AGZ_NS_END(Atrc)
