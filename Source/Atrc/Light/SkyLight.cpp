#include <Atrc/Light/SkyLight.h>

AGZ_NS_BEG(Atrc)

SkyLight::SkyLight(const Spectrum &topAndBottom)
    : SkyLight(topAndBottom, topAndBottom)
{

}

SkyLight::SkyLight(const Spectrum &top, const Spectrum &bottom)
    : top_(top), bottom_(bottom), worldRadius_(0.0)
{

}

void SkyLight::PreprocessScene(const Scene &scene)
{
    AGZ_ASSERT(!worldRadius_);

    auto &wb = scene.GetWorldBound();
    if(wb.IsEmpty())
    {
        worldRadius_ = 1.0;
        return;
    }

    worldCentre_ = 0.5 * (wb.high - wb.low);

    auto hd = wb.high - worldCentre_;
    worldRadius_ = 1.01 * hd.Length();
}

LightSampleToResult SkyLight::SampleLi(const SurfacePoint &sp) const
{
    // 只在sp的正半球上采样
    Real u = Rand(), v = Rand();
    auto [sam, pdf] = AGZ::Math::DistributionTransform::
        ZWeightedOnUnitHemisphere<Real>::Transform({ u, v });
    sam = sp.geoLocal.Local2World(sam).Normalize();

    float topWeight = 0.5f * float(sam.z) + 0.5f;

    LightSampleToResult ret;
    ret.pos      = sp.pos + 2.0 * sam * worldRadius_;
    ret.wi       = sam;
    ret.radiance = topWeight * top_ + (1 - topWeight) * bottom_;
    ret.pdf      = pdf;

    return ret;
}

Real SkyLight::SampleLiPDF(const Vec3 &pos, const SurfacePoint &dst, bool posOnLight) const
{
    AGZ_ASSERT(!posOnLight);

    Vec3 lwi = dst.geoLocal.World2Local(pos - dst.pos).Normalize();
    return AGZ::Math::DistributionTransform::
        ZWeightedOnUnitHemisphere<Real>::PDF(lwi);
}

bool SkyLight::IsDeltaPosition() const
{
    return false;
}

bool SkyLight::IsDeltaDirection() const
{
    return false;
}

bool SkyLight::IsDelta() const
{
    return false;
}

Spectrum SkyLight::AreaLe(const SurfacePoint &sp) const
{
    return Spectrum();
}

Spectrum SkyLight::NonareaLe(const Ray &r) const
{
    float topWeight = 0.5f * float(r.dir.z) + 0.5f;
    return topWeight * top_ + (1 - topWeight) * bottom_;
}

Spectrum SkyLight::Power() const
{
    return float(worldRadius_ * worldRadius_) * float(PI)
         * 0.5f * (top_ + bottom_);
}

AGZ_NS_END(Atrc)
