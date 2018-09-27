#include "PointLight.h"

AGZ_NS_BEG(Atrc)

PointLight::PointLight(const Spectrum &intensity, const Vec3r &position)
    : intensity_(intensity), position_(position)
{

}

bool PointLight::IsDeltaLight() const
{
    return true;
}

LightSample PointLight::SampleTo(const Vec3r &dst) const
{
    return { (position_ - dst).Normalize(), Real(1) };
}

AGZ_NS_END(Atrc)
