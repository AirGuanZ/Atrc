#include <Atrc/Light/DirectionalLight.h>

AGZ_NS_BEG(Atrc)

DirectionalLight::DirectionalLight(const Vec3 &dir, const Spectrum &radiance)
    : dir_(dir.Normalize()), radiance_(radiance), worldRadius_(0.0)
{

}

void DirectionalLight::PreprocessScene(const Scene &scene)
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

LightSampleToResult DirectionalLight::SampleLi(const SurfacePoint &sp) const
{
    AGZ_ASSERT(worldRadius_);

    LightSampleToResult ret;
    ret.pos      = sp.pos - 2 * worldRadius_ * dir_;
    ret.wi       = -dir_;
    ret.radiance = radiance_;
    ret.pdf      = 1;

    return ret;
}

Real DirectionalLight::SampleLiPDF([[maybe_unused]] const Vec3 &pos, [[maybe_unused]] const SurfacePoint &dst, [[maybe_unused]] bool posOnLight) const
{
    AGZ_ASSERT(!posOnLight);
    return 0.0;
}

bool DirectionalLight::IsDeltaPosition() const
{
    return false;
}

bool DirectionalLight::IsDeltaDirection() const
{
    return true;
}

bool DirectionalLight::IsDelta() const
{
    return true;
}

Spectrum DirectionalLight::AreaLe([[maybe_unused]] const SurfacePoint &sp) const
{
    return Spectrum();
}

bool DirectionalLight::IgnoreFirstMedium() const
{
    return true;
}

Spectrum DirectionalLight::NonareaLe([[maybe_unused]] const Ray &r) const
{
    return Spectrum();
}

Spectrum DirectionalLight::Power() const
{
    return radiance_ * float(PI * worldRadius_ * worldRadius_);
}

AGZ_NS_END(Atrc)
