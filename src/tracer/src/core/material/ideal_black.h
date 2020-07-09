#pragma once

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>

AGZ_TRACER_BEGIN

class IdealBlack : public Material
{
    class IdealBlackBSDF : public BSDF
    {
    public:

        FSpectrum eval(
            const FVec3 &, const FVec3 &, TransMode, uint8_t) const noexcept override
        {
            return FSpectrum();
        }

        BSDFSampleResult sample(
            const FVec3 &, TransMode mode, const Sample3&, uint8_t) const noexcept override
        {
            return BSDF_SAMPLE_RESULT_INVALID;
        }

        real pdf(const FVec3 &, const FVec3 &, uint8_t) const noexcept override
        {
            return 0;
        }

        FSpectrum albedo() const noexcept override
        {
            return FSpectrum();
        }

        bool is_delta() const noexcept override
        {
            return false;
        }

        bool has_diffuse_component() const noexcept override
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
