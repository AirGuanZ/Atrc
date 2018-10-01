#pragma once

#include <Atrc/Common.h>
#include <Atrc/Material/BxDF.h>
#include <Atrc/Material/Material.h>

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

class AmbientMaterial
    : ATRC_IMPLEMENTS Material,
      ATRC_PROPERTY AGZ::Uncopiable
{
    Spectrum color_;

public:

    explicit AmbientMaterial(const Spectrum &color);

    RC<BxDF> GetBxDF(const Intersection &inct) const override;
};

AGZ_NS_END(Atrc)
