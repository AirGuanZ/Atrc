#include <agz/tracer_cshader/cshader.h>

AGZ_TRACER_BEGIN

class OrenNayarBSDF : public CShaderBSDF
{
    Coord geometry_coord_;
    Coord shading_coord_;
    Spectrum albedo_;
    real A_ = 0;
    real B_ = 0;

public:

    OrenNayarBSDF(const AGZT_COperations *c_oprs, const Coord &geometry_coord, const Coord &shading_coord, const Spectrum &albedo, real sigma)
        : CShaderBSDF(c_oprs), geometry_coord_(geometry_coord), shading_coord_(shading_coord), albedo_(albedo)
    {
        real sigma2 = sigma * sigma;
        A_ = 1 - real(0.5) * sigma2 / (sigma2 + real(0.33));
        B_ = real(0.45) * sigma2 / (sigma2 + real(0.09));
    }

    Spectrum eval(const Vec3 &in_dir, const Vec3 &out_dir, bool) const noexcept override
    {
        Vec3 local_in  = shading_coord_.global_to_local(in_dir).normalize();
        Vec3 local_out = shading_coord_.global_to_local(out_dir).normalize();
        if(local_in.z <= 0 || local_out.z <= 0)
            return { };

        real theta_i = std::acos(local_angle::cos_theta(local_in));
        real theta_o = std::acos(local_angle::cos_theta(local_out));
        auto [beta, alpha] = std::minmax(theta_i, theta_o);

        real phi_i = local_angle::phi(local_in);
        real phi_o = local_angle::phi(local_out);

        auto ret = albedo_ / PI_r * (A_ + B_ * (std::max)(real(0), std::cos(phi_o - phi_i)) * std::sin(alpha) * std::tan(beta));
        ret *= local_angle::normal_corr_factor(geometry_coord_, shading_coord_, in_dir);
        return ret;
    }

    SampleResult sample(const Vec3 &dir, bool is_importance, const Sample3 &sam) const noexcept override
    {
        if(!shading_coord_.in_positive_z_hemisphere(dir))
            return SampleResult{};

        auto [local_in, pdf] = math::distribution::zweighted_on_hemisphere(sam.u, sam.v);
        if(pdf < EPS)
            return SampleResult{};

        SampleResult ret;
        ret.dir           = shading_coord_.local_to_global(local_in).normalize();
        ret.f             = eval(ret.dir, dir, is_importance);
        ret.pdf           = pdf;
        ret.is_importance = is_importance;
        ret.is_delta      = false;

        ret.f *= local_angle::normal_corr_factor(geometry_coord_, shading_coord_, ret.dir);
        return ret;
    }

    real pdf(const Vec3 &in_dir, const Vec3 &out_dir, bool) const noexcept override
    {
        if(!shading_coord_.in_positive_z_hemisphere(in_dir) || !shading_coord_.in_positive_z_hemisphere(out_dir))
            return 0;
        Vec3 local_in = shading_coord_.global_to_local(in_dir).normalize();
        return math::distribution::zweighted_on_hemisphere_pdf(local_in.z);
    }

    Spectrum albedo() const noexcept override
    {
        return albedo_;
    }
};

class OrenNayar : public CShaderMaterial
{
    AGZT_TextureHandle albedo_ = nullptr;
    AGZT_TextureHandle sigma_  = nullptr;

public:

    void initialize(const ConfigGroup &params, AGZT_InitContextHandle init_ctx) override
    {
        albedo_ = create_texture(params.child_group("albedo"), init_ctx);
        sigma_  = create_texture(params.child_group("sigma"),  init_ctx);
    }

    CShaderBSDF *shade(const Intersection &inct, AGZT_ArenaHandle arena) const override
    {
        auto albedo = sample_spectrum(albedo_, inct.uv);
        real sigma  = sample_real(sigma_, inct.uv);
        return new OrenNayarBSDF(c_oprs_, inct.geometry_coord, inct.user_coord, albedo, sigma);
    }
};

CShaderMaterial *new_material()
{
    return new OrenNayar;
}

AGZ_TRACER_END
