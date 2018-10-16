#pragma once

#include <Atrc/Material/Material.h>
#include "../../../../Utils/Src/Texture/Sampler.h"

AGZ_NS_BEG(Atrc)

namespace SamplingStrategy
{
    class Nearest
    {
    public:

        static constexpr Texture2DSampleType GetSamplingStrategy()
        {
            return Texture2DSampleType::Nearest;
        }
    };

    class Linear
    {
    public:

        static constexpr Texture2DSampleType GetSamplingStrategy()
        {
            return Texture2DSampleType::Linear;
        }
    };

    class Dynamic
    {
        Texture2DSampleType type_ = Texture2DSampleType::Nearest;

    public:

        Texture2DSampleType GetSamplingStrategy() const
        {
            return type_;
        }

        void SetSamplingStrategy(Texture2DSampleType type)
        {
            type_ = type;
        }
    };
}

template<typename M, typename Sampling = SamplingStrategy::Nearest,
         std::enable_if_t<std::is_base_of_v<Material, M>, int> = 0>
class TextureMultiplier
    : ATRC_IMPLEMENTS Material,
      ATRC_PROPERTY AGZ::Uncopiable,
      public Sampling
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

        Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override
        {
            return coef_ * bxdf_->Eval(wi, wo);
        }

        Real PDF(const Vec3r &wo, const Vec3r &sample) const override
        {
            return bxdf_->PDF(wo, sample);
        }

        Option<BxDFSample> Sample(const Vec3r &wo) const override
        {
            auto ret = bxdf_->Sample(wo);
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
        switch(Sampling::GetSamplingStrategy())
        {
        case Texture2DSampleType::Nearest:
            return NewRC<TextureMultiplierBxDF>(
                m_.GetBxDF(inct, matParam), AGZ::Tex::NearestSampler::Sample(tex_, matParam));
        case Texture2DSampleType::Linear:
            return NewRC<TextureMultiplierBxDF>(
                m_.GetBxDF(inct, matParam), AGZ::Tex::LinearSampler::Sample(tex_, matParam));
        default:
            AGZ::Unreachable();
        }
    }
};

AGZ_NS_END(Atrc)
