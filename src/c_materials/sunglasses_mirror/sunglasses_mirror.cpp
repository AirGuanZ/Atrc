#include <agz/tracer_cshader/c_interface.h>

AGZ_TRACER_BEGIN

namespace
{
    
    class MirrorSunglassesBSDF : public CShaderBSDF
    {
        Coord geometry_coord_;
        Coord shading_coord_;
        AGZT_FresnelPointHandle fresnel_point_;

        Spectrum view_color1_;
        Spectrum view_color2_;

    public:

        MirrorSunglassesBSDF(const AGZT_COperations *c_oprs,
                             const Coord &geometry_coord, const Coord &shading_coord,
                             AGZT_FresnelPointHandle fresnel_point,
                             const Spectrum &view_color1, const Spectrum &view_color2)
            : CShaderBSDF(c_oprs), geometry_coord_(geometry_coord), shading_coord_(shading_coord),
              fresnel_point_(fresnel_point), view_color1_(view_color1), view_color2_(view_color2)
        {

        }

        Spectrum eval(const Vec3 &wi, const Vec3 &wo, bool is_importance) const noexcept override
        {
            return Spectrum();
        }

        SampleResult sample(const Vec3 &wo, bool is_importance, const Sample3 &sam) const noexcept override
        {
            Vec3 local_out = shading_coord_.global_to_local(wo);
            if(local_out.z <= 0)
                return SampleResult{};

            Vec3 nwo = local_out.normalize();
            real color_t = math::clamp<real>(local_angle::theta(nwo) / (PI_r / 2), 0, 1);
            Spectrum color = math::mix(view_color1_, view_color2_, color_t);

            SampleResult ret;
            ret.dir           = shading_coord_.local_to_global(Vec3(0, 0, 2 * nwo.z) - nwo);
            ret.pdf           = 1;
            ret.f             = color * eval_fresnel_point(fresnel_point_, local_angle::cos_theta(nwo)) / std::abs(nwo.z);
            ret.is_importance = is_importance;
            ret.is_delta      = true;

            ret.f *= local_angle::normal_corr_factor(geometry_coord_, shading_coord_, ret.dir);
            return ret;
        }

        real pdf(const Vec3 &wi, const Vec3 &wo, bool is_importance) const noexcept override
        {
            return 0;
        }

        Spectrum albedo() const noexcept override
        {
            return real(0.5) * (view_color1_ + view_color2_);
        }

        bool is_delta() const noexcept override
        {
            return true;
        }

        bool is_black() const noexcept override
        {
            return !view_color1_ && !view_color2_;
        }
    };

} // namespace anonymous

class MirrorSunglasses : public CShaderMaterial
{
    AGZT_TextureHandle color1_  = nullptr;
    AGZT_TextureHandle color2_  = nullptr;
    AGZT_FresnelHandle fresnel_ = nullptr;

public:

    void initialize(const ConfigGroup &params, AGZT_InitContextHandle init_ctx) override
    {
        color1_ = create_texture(params.child_group("color1"), init_ctx);
        color2_ = create_texture(params.child_group("color2"), init_ctx);
        fresnel_ = create_fresnel(params.child_group("fresnel"), init_ctx);
    }

    CShaderBSDF *shade(const Intersection &inct, AGZT_ArenaHandle arena) const override
    {
        auto color1 = sample_spectrum(color1_, inct.uv);
        auto color2 = sample_spectrum(color2_, inct.uv);
        auto fresnel_point = create_fresnel_point(fresnel_, inct.uv, arena);
        return new MirrorSunglassesBSDF(c_oprs_, inct.geometry_coord, inct.user_coord, fresnel_point, color1, color2);
    }
};

CShaderMaterial *new_material()
{
    return new MirrorSunglasses;
}

AGZ_TRACER_END
