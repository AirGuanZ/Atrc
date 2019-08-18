#include <agz/tracer_cshader/c_interface.h>

AGZ_TRACER_BEGIN

class IdealDiffuseBSDF : public CShaderBSDF
{
    Coord coord_;
    Spectrum albedo_;

public:

    IdealDiffuseBSDF(const Coord &coord, const Spectrum &albedo)
        : coord_(coord), albedo_(albedo)
    {
        
    }

    Spectrum eval(const Vec3 &in_dir, const Vec3 &out_dir, bool) const noexcept override
    {
        // IMPROVE: fix black fringes
        Vec3 local_in = coord_.global_to_local(in_dir);
        Vec3 local_out = coord_.global_to_local(out_dir);
        if(local_in.z <= 0 || local_out.z <= 0)
            return { };
        return albedo_ / PI_r;
    }

    real proj_wi_factor(const Vec3 &wi) const noexcept override
    {
        return std::abs(cos(wi, coord_.z));
    }

    SampleResult sample(const Vec3 &dir, bool is_importance, const Sample3 &sam) const noexcept override
    {
        if(!coord_.in_positive_z_hemisphere(dir))
            return SampleResult{};

        auto[local_in, pdf] = math::distribution::zweighted_on_hemisphere(sam.u, sam.v);
        if(pdf < EPS)
            return SampleResult{};

        SampleResult ret;
        ret.dir = coord_.local_to_global(local_in).normalize();
        ret.f = albedo_ / PI_r;
        ret.pdf = pdf;
        ret.is_importance = is_importance;
        ret.is_delta = false;

        return ret;
    }

    real pdf(const Vec3 &in_dir, const Vec3 &out_dir, bool) const noexcept override
    {
        if(!coord_.in_positive_z_hemisphere(in_dir) || !coord_.in_positive_z_hemisphere(out_dir))
            return 0;
        Vec3 local_in = coord_.global_to_local(in_dir).normalize();
        return math::distribution::zweighted_on_hemisphere_pdf(local_in.z);
    }

    Spectrum albedo() const noexcept override
    {
        return albedo_;
    }
};

class IdealDiffuse : public CShaderMaterial
{
    AGZTTextureHandle albedo_ = -1;

public:

    void initialize(const ConfigGroup &params) override
    {
        albedo_ = create_texture(params.child_group("albedo"));
    }

    CShaderBSDF *shade(const Intersection &inct) const override
    {
        auto albedo = sample_spectrum(albedo_, inct.uv);
        return new IdealDiffuseBSDF(inct.user_coord, albedo);
    }
};

CShaderMaterial *new_material()
{
    return new IdealDiffuse;
}

AGZ_TRACER_END
