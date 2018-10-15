#include <Atrc/Material/MatOpr.h>

AGZ_NS_BEG(Atrc)

namespace
{
    class BxDFOpr_Add
        : ATRC_IMPLEMENTS BxDF
    {
        RC<BxDF> lhs_, rhs_;
        Real lProb_;

    public:

        BxDFOpr_Add(RC<BxDF> lhs, RC<BxDF> rhs, Real lProb);

        Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override;

        Option<BxDFSample> Sample(const Vec3r &wi) const override;

        Spectrum AmbientRadiance(const Intersection &inct) const override;

        const BxDF *GetLeafBxDF() const override;
    };

    BxDFOpr_Add::BxDFOpr_Add(RC<BxDF> lhs, RC<BxDF> rhs, Real lProb)
        : lhs_(std::move(lhs)), rhs_(std::move(rhs)), lProb_(lProb)
    {

    }

    Spectrum BxDFOpr_Add::Eval(const Vec3r &wi, const Vec3r &wo) const
    {
        return (Rand() < lProb_ ? lhs_ : rhs_)->Eval(wi, wo);
    }

    Option<BxDFSample> BxDFOpr_Add::Sample(const Vec3r &wi) const
    {
        return (Rand() < lProb_ ? lhs_ : rhs_)->Sample(wi);
    }

    Spectrum BxDFOpr_Add::AmbientRadiance(const Intersection &inct) const
    {
        return (SS(lProb_)      * lhs_->AmbientRadiance(inct)
             + (1 - SS(lProb_)) * rhs_->AmbientRadiance(inct));
    }

    const BxDF *BxDFOpr_Add::GetLeafBxDF() const
    {
        return (Rand() < lProb_ ? lhs_ : rhs_)->GetLeafBxDF();
    }
}

MatOpr_Add::MatOpr_Add(RC<Material> lhs, RC<Material> rhs, Real weightL)
    : lhs_(std::move(lhs)), rhs_(std::move(rhs)), lProb_(weightL)
{
    AGZ_ASSERT(0.0 <= lProb_ && lProb_ <= 1.0);
}

RC<BxDF> MatOpr_Add::GetBxDF(const Intersection &inct, const Vec2r &matParam) const
{
    return NewRC<BxDFOpr_Add>(
        lhs_->GetBxDF(inct, matParam), rhs_->GetBxDF(inct, matParam), lProb_);
}

AGZ_NS_END(Atrc)
