#pragma once

#include <Atrc/Common.h>
#include <Atrc/Material/Material.h>

AGZ_NS_BEG(Atrc)

class MatOpr_Add
    : ATRC_IMPLEMENTS Material
{
    RC<Material> lhs_, rhs_;
    Real lProb_;

public:

    MatOpr_Add(RC<Material> lhs, RC<Material> rhs, Real weightL);

    RC<BxDF> GetBxDF(const Intersection &inct, const Vec2r &matParam) const override;
};

AGZ_NS_END(Atrc)
