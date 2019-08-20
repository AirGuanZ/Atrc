#pragma once

#include <agz/tracer_utility/shader_c_api.h>

AGZ_TRACER_BEGIN

class CShaderBSDF : public misc::uncopyable_t
{
public:

    struct SampleResult
    {
        Vec3 dir;
        Spectrum f;
        real pdf           = 1;
        bool is_importance = false;
        bool is_delta      = false;
    };

    virtual ~CShaderBSDF() = default;

    virtual Spectrum eval(const Vec3 &wi, const Vec3 &wo, bool is_importance) const noexcept = 0;

    virtual real proj_wi_factor(const Vec3 &wi) const noexcept = 0;

    virtual SampleResult sample(const Vec3 &wo, bool is_importance, const Sample3 &sam) const noexcept = 0;

    virtual real pdf(const Vec3 &wi, const Vec3 &wo, bool is_importance) const noexcept = 0;

    virtual Spectrum albedo() const noexcept = 0;

    virtual bool is_delta() const noexcept { return false; }

    virtual bool is_black() const noexcept { return false; }
};

class CShaderMaterial : public misc::uncopyable_t
{
    AGZT_COperations *c_oprs_ = nullptr;

protected:

    AGZTTextureHandle create_texture(const ConfigGroup &params);

    real sample_real(AGZTTextureHandle tex, const Vec2 &uv) const;

    Spectrum sample_spectrum(AGZTTextureHandle tex, const Vec2 &uv) const;

public:

    void _set_c_oprs(AGZT_COperations *c_oprs) noexcept { c_oprs_ = c_oprs; }

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

    virtual void initialize(const ConfigGroup &params) = 0;

    virtual CShaderBSDF *shade(const Intersection &inct) const = 0;
};

// 用户通过实现该函数来实现材质
CShaderMaterial *new_material();

AGZ_TRACER_END
