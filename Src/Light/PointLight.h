#pragma once

#include "../Common.h"
#include "../Core/Light.h"
#include "../Core/Spectrum.h"

AGZ_NS_BEG(Atrc)

class PointLight
    : ATRC_IMPLEMENTS Light,
      ATRC_PROPERTY AGZ::Uncopiable
{
    Spectrum intensity_;
    Vec3r position_;

public:

    explicit PointLight(const Spectrum &intensity, const Vec3r &position);

    bool IsDeltaLight() const override;

    LightSample SampleTo(const Vec3r &dst) const override;
};

AGZ_NS_END(Atrc)
