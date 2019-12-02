#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/fresnel.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture2d.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

namespace
{
    class MirrorBSDF : public LocalBSDF
    {
        const FresnelPoint *fresnel_point_;
        Spectrum rc_;

    public:

        MirrorBSDF(const Coord &geometry_coord, const Coord &shading_coord,
                   const FresnelPoint *fresnel_point, const Spectrum &rc) noexcept
            : LocalBSDF(geometry_coord, shading_coord), fresnel_point_(fresnel_point), rc_(rc)
        {
            
        }

        Spectrum eval(const Vec3 &, const Vec3 &, TransportMode) const noexcept override
        {
            return Spectrum();
        }

        BSDFSampleResult sample(const Vec3 &out_dir, TransportMode transport_mode, const Sample3 &sam) const noexcept override
        {
            Vec3 local_out = shading_coord_.global_to_local(out_dir);
            if(local_out.z <= 0)
                return BSDF_SAMPLE_RESULT_INVALID;
            
            Vec3 nwo = local_out.normalize();
            BSDFSampleResult ret;
            ret.dir      = shading_coord_.local_to_global(Vec3(0, 0, 2 * nwo.z) - nwo);
            ret.pdf      = 1;
            ret.f        = fresnel_point_->eval(nwo.z) * rc_ / std::abs(nwo.z);
            ret.mode     = transport_mode;
            ret.is_delta = true;

            ret.f *= local_angle::normal_corr_factor(geometry_coord_, shading_coord_, ret.dir);
            return ret;
        }

        real pdf(const Vec3 &in_dir, const Vec3 &out_dir, TransportMode transport_mode) const noexcept override
        {
            return 0;
        }

        Spectrum albedo() const noexcept override
        {
            return rc_;
        }
    };
}

class Mirror : public Material
{
    std::shared_ptr<const Fresnel> fresnel_;
    std::shared_ptr<const Texture2D> rc_map_;

public:

    void initialize(std::shared_ptr<const Texture2D> color_map, std::shared_ptr<const Fresnel> fresnel)
    {
        rc_map_ = color_map;
        fresnel_ = fresnel;
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        auto fresnel_point = fresnel_->get_point(inct.uv, arena);
        auto rc = rc_map_->sample_spectrum(inct.uv);

        ShadingPoint ret;

        auto bsdf = arena.create<MirrorBSDF >(inct.geometry_coord, inct.user_coord, fresnel_point, rc);
        ret.bsdf = bsdf;
        ret.shading_normal = inct.user_coord.z;

        return ret;
    }
};

std::shared_ptr<Material> create_mirror(
    std::shared_ptr<const Texture2D> color_map,
    std::shared_ptr<const Fresnel> fresnel)
{
    auto ret = std::make_shared<Mirror>();
    ret->initialize(color_map, fresnel);
    return ret;
}

AGZ_TRACER_END
