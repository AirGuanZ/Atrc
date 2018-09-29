#pragma once

#include "../Common.h"
#include "../Core/Material.h"

AGZ_NS_BEG(Atrc)

class IdealGlassMaterial;

class IdealGlassBxDF
    : ATRC_IMPLEMENTS BxDF,
      ATRC_PROPERTY AGZ::Uncopiable
{
    const IdealGlassMaterial *material_;

public:

    explicit IdealGlassBxDF(const IdealGlassMaterial *material);

    BxDFType GetType() const override;

    Spectrum Eval(
        const EntityIntersection &inct, const Vec3r &wi, const Vec3r &wo) const override;

    Option<BxDFSample> Sample(
        const EntityIntersection &inct, const Vec3r &wo, SampleSeq2D &samSeq,
        BxDFType type) const override;

    Real PDF(
        const EntityIntersection &inct, const Vec3r &samDir, const Vec3r &wo) const override;
};

class IdealGlassMaterial
    : ATRC_IMPLEMENTS Material,
      ATRC_PROPERTY AGZ::Uncopiable
{
    friend class IdealGlassBxDF;

    Spectrum reflectionColor_;
    Spectrum transmissionColor_;
    Real refractivity_;

public:

    explicit IdealGlassMaterial(const Spectrum &refColor, const Spectrum &transColor,
                                Real refractivity = Real(1.55));

    RC<BxDF> GetBxDF(const SurfaceLocal &sl) const override;
};

AGZ_NS_END(Atrc)
