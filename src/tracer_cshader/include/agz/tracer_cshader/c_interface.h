#pragma once

#include <agz/tracer_utility/shader_c_api.h>

AGZ_TRACER_BEGIN

class CShaderBSDF : public misc::uncopyable_t
{
protected:

    const AGZT_COperations *c_oprs_ = nullptr;

    AGZT_TextureHandle create_texture(const ConfigGroup &params, AGZT_InitContextHandle init_ctx);

    real sample_real(AGZT_TextureHandle tex, const Vec2 &uv) const;

    Spectrum sample_spectrum(AGZT_TextureHandle tex, const Vec2 &uv) const;

    AGZT_FresnelHandle create_fresnel(const ConfigGroup &params, AGZT_InitContextHandle init_ctx);

    AGZT_FresnelPointHandle create_fresnel_point(AGZT_FresnelHandle fresnel, const Vec2 &uv, AGZT_ArenaHandle arena) const;

    Spectrum eval_fresnel_point(AGZT_FresnelPointHandle fresnel_point, real cos_theta_i) const;

    real get_fresnel_point_eta_i(AGZT_FresnelPointHandle fresnel_point) const;

    real get_fresnel_point_eta_o(AGZT_FresnelPointHandle fresnel_point) const;

public:

    struct SampleResult
    {
        Vec3 dir;
        Spectrum f;
        real pdf           = 1;
        bool is_importance = false;
        bool is_delta      = false;
    };

    explicit CShaderBSDF(const AGZT_COperations *c_oprs) noexcept
        : c_oprs_(c_oprs)
    {
        
    }

    virtual ~CShaderBSDF() = default;

    virtual Spectrum eval(const Vec3 &wi, const Vec3 &wo, bool is_importance) const noexcept = 0;

    virtual SampleResult sample(const Vec3 &wo, bool is_importance, const Sample3 &sam) const noexcept = 0;

    virtual real pdf(const Vec3 &wi, const Vec3 &wo, bool is_importance) const noexcept = 0;

    virtual Spectrum albedo() const noexcept = 0;

    virtual bool is_delta() const noexcept { return false; }

    virtual bool is_black() const noexcept { return false; }
};

class CShaderMaterial : public misc::uncopyable_t
{
protected:

    const AGZT_COperations *c_oprs_ = nullptr;

    AGZT_TextureHandle create_texture(const ConfigGroup &params, AGZT_InitContextHandle init_ctx);

    real sample_real(AGZT_TextureHandle tex, const Vec2 &uv) const;

    Spectrum sample_spectrum(AGZT_TextureHandle tex, const Vec2 &uv) const;

    AGZT_FresnelHandle create_fresnel(const ConfigGroup &params, AGZT_InitContextHandle init_ctx);

    AGZT_FresnelPointHandle create_fresnel_point(AGZT_FresnelHandle fresnel, const Vec2 &uv, AGZT_ArenaHandle arena) const;

    Spectrum eval_fresnel_point(AGZT_FresnelPointHandle fresnel_point, real cos_theta_i) const;

    real get_fresnel_point_eta_i(AGZT_FresnelPointHandle fresnel_point) const;

    real get_fresnel_point_eta_o(AGZT_FresnelPointHandle fresnel_point) const;

public:

    void _set_c_oprs(const AGZT_COperations *c_oprs) noexcept { c_oprs_ = c_oprs; }

    struct Intersection
    {
        Vec3 pos;
        Vec2 uv;
        Coord geometry_coord;
        Coord user_coord;
        real t = -1;
        Vec3 wr;
    };

    virtual ~CShaderMaterial() = default;

    virtual void initialize(const ConfigGroup &params, AGZT_InitContextHandle init_ctx) = 0;

    virtual CShaderBSDF *shade(const Intersection &inct, AGZT_ArenaHandle arena) const = 0;
};

// 用户通过实现该函数来实现材质
CShaderMaterial *new_material();

AGZ_TRACER_END
