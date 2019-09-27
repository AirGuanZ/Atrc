#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/fresnel.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture.h>

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
    const Fresnel *fresnel_ = nullptr;
    const Texture *rc_map_  = nullptr;

public:

    explicit Mirror(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

    static std::string description()
    {
        return R"___(
mirror [Material]
    fresnel   [Fresnel] fresnel object
    color_map [Texture] reflection color map
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        fresnel_ = FresnelFactory.create(params.child_group("fresnel"), init_ctx);
        rc_map_ = TextureFactory.create(params.child_group("color_map"), init_ctx);

        AGZ_HIERARCHY_WRAP("in initializing ideal_mirror material object")
    }

    void initialize(const Texture *color_map, const Fresnel *fresnel)
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

        return ret;
    }
};

Material *create_mirror(
    const Texture *color_map,
    const Fresnel *fresnel,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<Mirror>(customed_flag);
    ret->initialize(color_map, fresnel);
    return ret;
}

AGZT_IMPLEMENTATION(Material, Mirror, "mirror")

AGZ_TRACER_END
