#pragma once

#include <Atrc/Lib/Core/Material.h>
#include <Atrc/Lib/Material/Utility/BxDF.h>

namespace Atrc
{
    
template<typename TBxDF>
class BxDF2BSDF : public BSDF
{
    static_assert(std::is_base_of_v<BxDF, TBxDF>);

    TBxDF bxdf_;

public:

    template<typename...Args>
    explicit BxDF2BSDF(Args&&...args)
        : bxdf_(std::forward<Args>(args)...)
    {
        
    }

    Spectrum GetBaseColor(BSDFType type) const noexcept override
    {
        return bxdf_.MatchType(type) ? bxdf_.GetBaseColor() : Spectrum();
    }

    Spectrum Eval(const CoordSystem &shd, const CoordSystem &geo, const Vec3 &wi, const Vec3 &wo, BSDFType type, bool star) const noexcept override
    {
        if(!bxdf_.MatchType(type))
            return Spectrum();
        Vec3 lwi = shd.World2Local(wi).Normalize(), lwo = shd.World2Local(wo).Normalize();
        return bxdf_.Eval(lwi, lwo, star);
    }

    std::optional<SampleWiResult> SampleWi(const CoordSystem &shd, const CoordSystem &geo, const Vec3 &wo, BSDFType type, bool star, const Vec3 &sample) const noexcept override
    {
        if(!bxdf_.MatchType(type))
            return std::nullopt;

        Vec3 lwo = shd.World2Local(wo).Normalize();
        auto ret = bxdf_.SampleWi(lwo, star, sample);
        if(!ret)
            return std::nullopt;

        ret->wi = shd.Local2World(ret->wi).Normalize();
        return ret;
    }

    Real SampleWiPDF(const CoordSystem &shd, const CoordSystem &geo, const Vec3 &wi, const Vec3 &wo, BSDFType type, bool star) const noexcept override
    {
        if(!bxdf_.MatchType(type))
            return 0;
        Vec3 lwi = shd.World2Local(wi).Normalize(), lwo = shd.World2Local(wo).Normalize();
        return bxdf_.SampleWiPDF(lwi, lwo, star);
    }
};

} // namespace Atrc
