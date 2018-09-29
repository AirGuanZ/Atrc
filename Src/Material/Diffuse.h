#pragma once

#include "../Common.h"
#include "../Core/Material.h"

AGZ_NS_BEG(Atrc)

class DiffuseBRDF
    : ATRC_IMPLEMENTS BxDF,
      ATRC_PROPERTY AGZ::Uncopiable
{
    Spectrum color_;

public:

    explicit DiffuseBRDF(const Spectrum &color);

    BxDFType GetType() const override;

    Spectrum Eval(
        const EntityIntersection &inct, const Vec3r &wi, const Vec3r &wo) const override;

    Option<BxDFSample> Sample(
        const EntityIntersection &inct, const Vec3r &wo, SampleSeq2D &samSeq,
        BxDFType type) const override;

    Real PDF(
        const EntityIntersection &inct, const Vec3r &samDir, const Vec3r &wo) const override;
};

class DiffuseMaterial
    : ATRC_IMPLEMENTS Material,
      ATRC_PROPERTY AGZ::Uncopiable
{
    Spectrum color_;

public:

    explicit DiffuseMaterial(const Spectrum &albedo);

    RC<BxDF> GetBxDF(const SurfaceLocal &sl) const override;
};

AGZ_NS_END(Atrc)
