#pragma once

#include <Atrc/Common.h>
#include <Atrc/Material/BxDF.h>
#include <Atrc/Material/Material.h>

AGZ_NS_BEG(Atrc)

class NormalBRDF
    : ATRC_IMPLEMENTS BxDF,
    ATRC_PROPERTY AGZ::Uncopiable
{
public:

    BxDFType GetType() const override;

    Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override;

    Option<BxDFSample> Sample(const Vec3r &wi, BxDFType type) const override;

    Spectrum AmbientRadiance(const Intersection &inct) const override;
};

class NormalMaterial
    : ATRC_IMPLEMENTS Material,
    ATRC_PROPERTY AGZ::Uncopiable
{
public:

    RC<BxDF> GetBxDF(const Intersection &inct, const Vec2r &matParam) const override;
};

AGZ_NS_END(Atrc)
