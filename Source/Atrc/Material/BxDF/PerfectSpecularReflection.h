#pragma once

#include <Atrc/Material/BxDF/BxDF.h>
#include <Atrc/Material/Fresnel.h>

AGZ_NS_BEG(Atrc)

template<typename F, std::enable_if_t<std::is_base_of_v<Fresnel, F>, int> = 0>
class PerfectSpecularReflection : public BxDF
{
    Spectrum rc_;
    F fresnel_;

public:

    template<typename...Args>
    explicit PerfectSpecularReflection(const Spectrum &rc, Args&&...fresnelArgs)
        : BxDF(BXDF_SPECULAR | BXDF_REFLECTION), rc_(rc), fresnel_(std::forward<Args>(fresnelArgs)...)
    {
        
    }

    Spectrum Eval(const Vec3 &wi, const Vec3 &wo) const override
    {
        return Spectrum();
    }

    Option<BxDFSampleWiResult> SampleWi(const Vec3 &wo, BxDFType type) const override
    {
        AGZ_ASSERT(Match(type_, type));

        if(wo.z <= 0.0)
            return None;

        AGZ_ASSERT(IsNormalized(wo));

        BxDFSampleWiResult ret;
        ret.wi   = 2 * wo.z * Vec3::UNIT_Z() - wo;
        ret.coef = rc_ * fresnel_.Eval(wo.z) / wo.z;
        ret.pdf  = 1.0;
        ret.type = BxDFType(BXDF_SPECULAR | BXDF_REFLECTION);

        return ret;
    }
};

AGZ_NS_END(Atrc)
