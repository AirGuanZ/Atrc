#pragma once

#include <Atrc/Common.h>
#include <Atrc/Material/Material.h>

AGZ_NS_BEG(Atrc)

class BxDFOpr_Add
    : ATRC_IMPLEMENTS BxDF
{
    RC<BxDF> lhs_, rhs_;
    Real lProb_;
    SS coef_;

public:

    BxDFOpr_Add(RC<BxDF> lhs, RC<BxDF> rhs, Real lProb, SS coef);

    BxDFType GetType() const override;

    Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override;

    Option<BxDFSample> Sample(const Vec3r &wi, BxDFType type) const override;

    Spectrum AmbientRadiance(const Intersection &inct) const override;
};

class MatOpr_Add
    : ATRC_IMPLEMENTS Material
{
    RC<Material> lhs_, rhs_;
    Real lProb_;
    SS coef_;

public:

    MatOpr_Add(RC<Material> lhs, RC<Material> rhs, Real wL, Real wR);

    RC<BxDF> GetBxDF(const Intersection &inct, const Vec2r &matParam) const override;
};

AGZ_NS_END(Atrc)
