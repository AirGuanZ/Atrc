#pragma once

#include "../Common.h"
#include "../Core/Material.h"
#include "../Core/Spectrum.h"

AGZ_NS_BEG(Atrc)

class PureColorBRDF
    : ATRC_IMPLEMENTS BxDF,
      ATRC_PROPERTY AGZ::Uncopiable
{
    Spectrum color_;

public:

    explicit PureColorBRDF(const Spectrum &color);

    BxDFType GetType() const override;

    Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override;

    Option<BxDFSample> Sample(
        const SurfaceLocal &sl, const Vec3r &wo, SampleSeq2D &samSeq,
        BxDFType type) const override;

    Real PDF(const Vec3r &wi, const Vec3r &wo) const override;

    Spectrum AmbientRadiance() const override;
};

class PureColorMaterial
    : ATRC_IMPLEMENTS Material,
      ATRC_PROPERTY AGZ::Uncopiable
{
    Spectrum color_;

public:

    explicit PureColorMaterial(const Spectrum &color);

    BxDF* GetBxDF(const SurfaceLocal &sl) const override;
};

AGZ_NS_END(Atrc)
