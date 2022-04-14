#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture2d.h>
#include <agz-utils/misc.h>

#include "./utility/fresnel_point.h"

AGZ_TRACER_BEGIN

namespace
{
    class MirrorBSDF final : public LocalBSDF
    {
        const ConductorPoint *fresnel_point_;
        FSpectrum rc_;

    public:

        MirrorBSDF(const FCoord &geometry_coord, const FCoord &shading_coord,
                   const ConductorPoint *fresnel_point, const FSpectrum &rc) noexcept
            : LocalBSDF(geometry_coord, shading_coord),
              fresnel_point_(fresnel_point), rc_(rc)
        {
            
        }

        FSpectrum eval(const FVec3 &, const FVec3 &, TransMode) const noexcept override
        {
            return FSpectrum();
        }

        BSDFSampleResult sample(const FVec3 &out_dir, TransMode, const Sample3 &sam) const noexcept override
        {
            const FVec3 local_out = shading_coord_.global_to_local(out_dir);
            if(local_out.z <= 0)
                return BSDF_SAMPLE_RESULT_INVALID;
            
            const FVec3 nwo = local_out.normalize();

            const FVec3 dir = shading_coord_.local_to_global(
                FVec3(0, 0, 2 * nwo.z) - nwo);

            const FSpectrum f = fresnel_point_->eval(nwo.z)
                             * rc_ / std::abs(nwo.z);

            const real norm_factor = local_angle::normal_corr_factor(
                geometry_coord_, shading_coord_, dir);

            return BSDFSampleResult(dir, f * norm_factor, 1, true);
        }

        BSDFBidirSampleResult sample_bidir(const FVec3 &wo, TransMode mode, const Sample3 &sam) const override
        {
            auto t = sample(wo, mode, sam);
            return BSDFBidirSampleResult(t.dir, t.f, t.pdf, t.pdf, t.is_delta);
        }

        real pdf(const FVec3 &in_dir, const FVec3 &out_dir) const noexcept override
        {
            return 0;
        }

        FSpectrum albedo() const noexcept override
        {
            return rc_;
        }

        bool is_delta() const noexcept override
        {
            return true;
        }

        bool has_diffuse_component() const noexcept override
        {
            return false;
        }
    };
}

class Mirror : public Material
{
    RC<const Texture2D> ior_;
    RC<const Texture2D> k_;
    RC<const Texture2D> rc_map_;

public:

    Mirror(
        RC<const Texture2D> color_map,
        RC<const Texture2D> ior,
        RC<const Texture2D> k)
    {
        rc_map_ = color_map;
        ior_    = ior;
        k_      = k;
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        const FSpectrum rc  = rc_map_->sample_spectrum(inct.uv);
        const FSpectrum ior = ior_   ->sample_spectrum(inct.uv);
        const FSpectrum k   = k_     ->sample_spectrum(inct.uv);

        const ConductorPoint *fresnel = arena.create_nodestruct<ConductorPoint>(
                                            ior, FSpectrum(1), k);

        ShadingPoint ret;

        const BSDF *bsdf = arena.create_nodestruct<MirrorBSDF>(
            inct.geometry_coord, inct.user_coord, fresnel, rc);
        ret.bsdf = bsdf;
        ret.shading_normal = inct.user_coord.z;

        return ret;
    }
};

RC<Material> create_mirror(
    RC<const Texture2D> color_map,
    RC<const Texture2D> eta,
    RC<const Texture2D> k)
{
    return newRC<Mirror>(color_map, eta, k);
}

AGZ_TRACER_END
