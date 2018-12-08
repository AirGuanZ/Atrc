#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class TextureScaler : public Material
{
    const Texture *tex_;
    const Material *mat_;

    class BSDFScaler : public BSDF
    {
        Spectrum scale_;
        BSDF *bsdf_;

    public:

        BSDFScaler(
            const Vec3 &shadingNormal, const LocalCoordSystem &geoLocal,
            const Spectrum &scale, BSDF *bsdf)
            : BSDF(shadingNormal, geoLocal), scale_(scale), bsdf_(bsdf)
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
            return Some(*ret);
        }

        Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo, BxDFType type) const override
        {
            AGZ_ASSERT(IsNormalized(wi));
            AGZ_ASSERT(IsNormalized(wo));
            return bsdf_->SampleWiPDF(wi, wo, type);
        }
    };

public:

    TextureScaler(const Texture *tex, const Material *mat)
        : tex_(tex), mat_(mat)
    {
        AGZ_ASSERT(tex && mat);
    }

    void Shade(const SurfacePoint &sp, ShadingPoint *dst, AGZ::ObjArena<> &arena) const override
    {
        mat_->Shade(sp, dst, arena);

        Spectrum scale = tex_->Sample(sp.usrUV);
        dst->bsdf = arena.Create<BSDFScaler>(
            dst->bsdf->GetShadingLocal().ez, dst->bsdf->GetGeometryLocal(), scale, dst->bsdf);
    }
};

AGZ_NS_END(Atrc)
