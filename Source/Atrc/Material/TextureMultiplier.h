#pragma once

#include <Atrc/Material/Material.h>

AGZ_NS_BEG(Atrc)

template<typename M, std::enable_if_t<std::is_base_of_v<Material, M>, int> = 0>
class TextureMultiplier
    : ATRC_IMPLEMENTS Material,
      ATRC_PROPERTY AGZ::Uncopiable
{
    const Tex2D<Color3f> &tex_;
    M m_;

    class TextureMultiplierBxDF
        : ATRC_IMPLEMENTS BxDF,
          ATRC_PROPERTY AGZ::Uncopiable
    {
        RC<BxDF> bxdf_;
        Spectrum coef_;

    public:

        TextureMultiplierBxDF(RC<BxDF> bxdf, const Spectrum &coef)
            : bxdf_(std::move(bxdf)), coef_(coef)
        {
            AGZ_ASSERT(bxdf_);
        }

        BxDFType GetType() const override
        {
            return bxdf_->GetType();
        }

        Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override
        {
            return coef_ * bxdf_->Eval(wi, wo);
        }

        Option<BxDFSample> Sample(const Vec3r &wi, BxDFType type) const override
        {
            auto ret = bxdf_->Sample(wi, type);
            if(ret)
                ret->coef *= coef_;
            return ret;
        }
    
        Spectrum AmbientRadiance(const Intersection &inct) const override
        {
            return coef_ * bxdf_->AmbientRadiance(inct);
        }
    };

public:

    template<typename...Args>
    explicit TextureMultiplier(const Tex2D<Color3f> &tex, Args&&...args)
        : tex_(tex), m_(std::forward<Args>(args)...)
    {
        
    }

    RC<BxDF> GetBxDF(const Intersection &inct, const Vec2r &matParam) const override
    {
        return NewRC<TextureMultiplierBxDF>(
            m_.GetBxDF(inct, matParam),
            AGZ::Tex::NearestSampler::Sample(tex_, matParam));
    }
};

AGZ_NS_END(Atrc)
