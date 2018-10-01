#pragma once

#include <Atrc/Common.h>
#include <Atrc/Material/BxDF.h>
#include <Atrc/Material/BxDFGen.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

class DiffuseBxDF
    : ATRC_IMPLEMENTS BxDF,
    ATRC_PROPERTY AGZ::Uncopiable
{
    CoordSys localCoord_;
    Spectrum color_;

public:

    explicit DiffuseBxDF(const Intersection &inct, const Spectrum &color);

    BxDFType GetType() const override;

    Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override;

    Option<BxDFSample> Sample(const Vec3r &wi, BxDFType type) const override;
};

class DiffuseGen
    : ATRC_IMPLEMENTS BxDFGenerator,
      ATRC_PROPERTY AGZ::Uncopiable
{
    Spectrum color_;

public:

    explicit DiffuseGen(const Spectrum &color);

    RC<BxDF> GetBxDF(const Intersection &inct) const override;
};

AGZ_NS_END(Atrc)
