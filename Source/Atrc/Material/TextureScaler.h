#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class NearestSampler
{
protected:

    static Spectrum SampleTexture(const AGZ::Texture2D<Spectrum> &tex, const Vec2 &uv)
    {
        return AGZ::NearestSampler::Sample(tex, uv);
    }
};

class LinearSampler
{
protected:

    static Spectrum SampleTexture(const AGZ::Texture2D<Spectrum> &tex, const Vec2 &uv)
    {
        return AGZ::LinearSampler::Sample(tex, uv);
    }
};

class DynamicSampler
{
public:

    enum Strategy
    {
        Linear,
        Nearest,
    };

    void SetSampleStrategy(Strategy strategy)
    {
        strategy_ = strategy;
    }

private:

    Strategy strategy_ = Linear;

protected:

    Spectrum SampleTexture(const AGZ::Texture2D<Spectrum> &tex, const Vec2 &uv) const
    {
        if(strategy_ == Linear)
            return AGZ::LinearSampler::Sample(tex, uv);
        return AGZ::NearestSampler::Sample(tex, uv);
    }
};

template<typename SampleStrategy = NearestSampler>
class TextureScaler : public Material, public SampleStrategy
{
    const AGZ::Texture2D<Spectrum> *tex_;
    const Material *mat_;

    class BSDFScaler : public BSDF
    {
        Spectrum scale_;
        BSDF *bsdf_;

    public:

        BSDFScaler(
            const LocalCoordSystem &shadingLocal, const LocalCoordSystem &geoLocal,
            const Spectrum &scale, BSDF *bsdf)
            : BSDF(shadingLocal, geoLocal), scale_(scale), bsdf_(bsdf)
        {
            AGZ_ASSERT(bsdf);
        }

        Spectrum Eval(const Vec3 &wi, const Vec3 &wo, BxDFType type) const override
        {
            return scale_ * bsdf_->Eval(wi, wo, type);
        }

        Option<BSDFSampleWiResult> SampleWi(const Vec3 &wo, BxDFType type) const override
        {
            auto ret = bsdf_->SampleWi(wo, type);
            if(ret)
                ret->coef *= scale_;
            return ret;
        }

        Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo, BxDFType type) const override
        {
            AGZ_ASSERT(IsNormalized(wi));
            AGZ_ASSERT(IsNormalized(wo));
            return bsdf_->SampleWiPDF(wi, wo, type);
        }
    };

public:

    TextureScaler(const AGZ::Texture2D<Spectrum> *tex, const Material *mat)
        : tex_(tex), mat_(mat)
    {
        AGZ_ASSERT(tex && mat);
    }

    void Shade(const SurfacePoint &sp, ShadingPoint *dst, AGZ::ObjArena<> &arena) const override
    {
        mat_->Shade(sp, dst, arena);

        Spectrum scale = SampleStrategy::SampleTexture(*tex_, sp.usrUV);
        dst->bsdf = arena.Create<BSDFScaler>(
            dst->bsdf->GetShadingLocal(), dst->bsdf->GetGeometryLocal(), scale, dst->bsdf);
    }
};

AGZ_NS_END(Atrc)
