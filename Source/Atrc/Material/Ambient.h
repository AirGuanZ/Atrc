#pragma once

#include <Atrc/Common.h>
#include <Atrc/Material/BxDF.h>

AGZ_NS_BEG(Atrc)

class AmbientBRDF
    : ATRC_IMPLEMENTS BxDF,
    ATRC_PROPERTY AGZ::Uncopiable
{
    Spectrum color_;

public:

    explicit AmbientBRDF(const Spectrum &color);

    BxDFType GetType() const override;

    Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override;

    Option<BxDFSample> Sample(const Vec3r &wi, BxDFType type) const override;

    Spectrum AmbientRadiance(const Intersection &inct) const override;
};

AGZ_NS_END(Atrc)
