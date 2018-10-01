#pragma once

#include <Atrc/Common.h>
#include <Atrc/Material/BxDF.h>
#include <Atrc/Material/Material.h>

AGZ_NS_BEG(Atrc)

class GlassBxDF
    : ATRC_IMPLEMENTS BxDF,
    ATRC_PROPERTY AGZ::Uncopiable
{
    Vec3r nor_;
    Spectrum reflColor_, refrColor_;
    Real refIdx_;

public:

    explicit GlassBxDF(
        const Intersection &inct, const Spectrum &reflColor, const Spectrum &refrColor, Real refIdx = 1.5);

    BxDFType GetType() const override;

    Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override;

    Option<BxDFSample> Sample(const Vec3r &wi, BxDFType type) const override;
};

class GlassMaterial
    : ATRC_IMPLEMENTS Material,
      ATRC_PROPERTY AGZ::Uncopiable
{
    Spectrum reflColor_, refrColor_;
    Real refIdx_;

public:

    explicit GlassMaterial(const Spectrum &reflRefrColor, Real refIdx = 1.5)
        : GlassMaterial(reflRefrColor, reflRefrColor, refIdx)
    {
        
    }

    GlassMaterial(const Spectrum &reflColor, const Spectrum &refrColor, Real refIdx = 1.5);

    RC<BxDF> GetBxDF(const Intersection &inct) const override;
};

AGZ_NS_END(Atrc)
