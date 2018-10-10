#pragma once

#include <Atrc/Common.h>
#include <Atrc/Material/Material.h>

AGZ_NS_BEG(Atrc)

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
