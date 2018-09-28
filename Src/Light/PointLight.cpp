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

LightSample PointLight::SampleTo(const Vec3r &dst, SampleSeq2D &samSeq) const
{
    Vec3r dis = position_ - dst;
    Real invLengthSquare = Real(1) / dis.LengthSquare();
    return {
        intensity_ * static_cast<decltype(intensity_.r)>(invLengthSquare),
        position_, dis, Real(1)
    };
}

uint32_t PointLight::SampleCount(uint32_t sampleLevel) const
{
    return 1;
}

AGZ_NS_END(Atrc)
