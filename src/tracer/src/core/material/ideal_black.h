#pragma once

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>

AGZ_TRACER_BEGIN

class IdealBlack final : public Material
{
    class IdealBlackBSDF : public BSDF
    {
    public:

        FSpectrum eval(const FVec3 &, const FVec3 &, TransMode) const override
        {
            return FSpectrum();
        }

        BSDFSampleResult sample(const FVec3 &, TransMode mode, const Sample3&) const override
        {
            return BSDF_SAMPLE_RESULT_INVALID;
        }

        BSDFBidirSampleResult sample_bidir(const FVec3 &wo, TransMode mode, const Sample3 &sam) const override
        {
            return BSDF_BIDIR_SAMPLE_RESULT_INVALID;
        }

        real pdf(const FVec3 &, const FVec3 &) const override
        {
            return 0;
        }

        FSpectrum albedo() const override
        {
            return FSpectrum();
        }

        bool is_delta() const override
        {
            return false;
        }

        bool has_diffuse_component() const override
        {
            return false;
        }
    };

    IdealBlackBSDF bsdf_;

public:

    ShadingPoint shade(const EntityIntersection &inct, Arena&) const override
    {
        ShadingPoint shd;
        shd.bsdf = IDEAL_BLACK_BSDF_INSTANCE();
        shd.shading_normal = inct.user_coord.z;
        return shd;
    }

    static const BSDF *IDEAL_BLACK_BSDF_INSTANCE()
    {
        static const IdealBlackBSDF bsdf;
        return &bsdf;
    }

    static const IdealBlack &IDEAL_BLACK_INSTANCE()
    {
        static IdealBlack ret;
        return ret;
    }
};

AGZ_TRACER_END
