#include <Atrc/Lib/Material/IdealScaler.h>

namespace Atrc
{

namespace
{
    class BSDF_Scaler : public BSDF
    {
        Spectrum scale_;
        const BSDF *internal_;

    public:

        BSDF_Scaler(const Spectrum &scale, const BSDF *internal) noexcept
            : scale_(scale), internal_(internal)
        {

        }
        
        Spectrum GetAlbedo(BSDFType type) const noexcept override
        {
            return scale_ * internal_->GetAlbedo(type);
        }

        Spectrum Eval(
            const CoordSystem &shd, const CoordSystem &geo,
            const Vec3 &wi, const Vec3 &wo, BSDFType type, bool star) const noexcept override
        {
            return scale_ * internal_->Eval(shd, geo, wi, wo, type, star);
        }

        std::optional<SampleWiResult> SampleWi(
            const CoordSystem &shd, const CoordSystem &geo,
            const Vec3 &wo, BSDFType type, bool star, const Vec3 &sample) const noexcept override
        {
            auto ret = internal_->SampleWi(shd, geo, wo, type, star, sample);
            if(ret)
                ret->coef *= scale_;
            return ret;
        }

        Real SampleWiPDF(
            const CoordSystem &shd, const CoordSystem &geo,
            const Vec3 &wi, const Vec3 &wo, BSDFType type, bool star) const noexcept override
        {
            return internal_->SampleWiPDF(shd, geo, wi, wo, type, star);
        }
    };
}

IdealScaler::IdealScaler(const Texture *scaleMap, const Material *internal) noexcept
    : scaleMap_(scaleMap), internal_(internal)
{

}

ShadingPoint IdealScaler::GetShadingPoint(const Intersection &inct, Arena &arena) const noexcept
{
    auto shd = internal_->GetShadingPoint(inct, arena);
    auto scale = scaleMap_->Sample(shd.uv);
    auto bsdf = arena.Create<BSDF_Scaler>(scale, shd.bsdf);
    shd.bsdf = bsdf;
    return shd;
}

} // namespace Atrc
