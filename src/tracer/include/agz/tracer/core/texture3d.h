#pragma once

#include <agz/tracer/common.h>
#include <agz/tracer/utility/config.h>

AGZ_TRACER_BEGIN

struct Texture3DCommonParams
{
    bool inv_u = false;
    bool inv_v = false;
    bool inv_w = false;
    Vec3i uvw_perm = Vec3i(0, 1, 2);
    FTransform3 transform;

    std::string wrap_u = "clamp";
    std::string wrap_v = "clamp";
    std::string wrap_w = "clamp";

    real inv_gamma = 1;

    FTransform3 full_transform() const;

    void from_params(const ConfigGroup &params);
};

/**
 * @brief 3d texture interface
 */
class Texture3D
{
protected:

    FTransform3 transform_;

    using WrapFuncPtr = real(*)(real);

    WrapFuncPtr wrapper_u_ = &wrap_clamp;
    WrapFuncPtr wrapper_v_ = &wrap_clamp;
    WrapFuncPtr wrapper_w_ = &wrap_clamp;

    real inv_gamma_ = 1;

    static real wrap_clamp(real x) noexcept;

    static real wrap_repeat(real x) noexcept;

    static real wrap_mirror(real x) noexcept;

    static WrapFuncPtr str_to_wrap_func(const std::string &str);

    void init_common_params(const Texture3DCommonParams &params);

    virtual FSpectrum sample_spectrum_impl(const FVec3 &uvw) const noexcept;

    virtual real sample_real_impl(const FVec3 &uvw) const noexcept;

public:

    virtual ~Texture3D() = default;

    /**
     * @brief sample spectrum value at uv
     */
    virtual FSpectrum sample_spectrum(const FVec3 &uvw) const noexcept;
    
    /**
     * @brief sample real value at uv
     */
    virtual real sample_real(const FVec3 &uvw) const noexcept;

    virtual int width() const noexcept = 0;

    virtual int height() const noexcept = 0;

    virtual int depth() const noexcept = 0;

    /**
     * @brief element-wise maximal spectrum value
     */
    virtual FSpectrum max_spectrum() const noexcept = 0;

    /**
     * @brief maximal real value
     */
    virtual real max_real() const noexcept = 0;
};

inline FTransform3 Texture3DCommonParams::full_transform() const
{
    using Trans4 = Mat4::left_transform;

    FTransform3 ret;
    
    if(inv_u)
        ret *= FTransform3(Trans4::translate(1, 0, 0) *
                           Trans4::scale(-1, 1, 1));
    if(inv_v)
        ret *= FTransform3(Trans4::translate(0, 1, 0) *
                           Trans4::scale(1, -1, 1));
    if(inv_w)
        ret *= FTransform3(Trans4::translate(0, 0, 1) *
                           Trans4::scale(1, 1, -1));

    auto perm_to_row = [&](int perm)
    {
        if(perm < 0 || perm >= 3)
            throw ObjectConstructionException("invalid uvw perm value");
        FVec3 row;
        row[perm] = 1;
        return row;
    };
    const FVec3 u_row = perm_to_row(uvw_perm.x);
    const FVec3 v_row = perm_to_row(uvw_perm.y);
    const FVec3 w_row = perm_to_row(uvw_perm.z);

    ret *= FTransform3(Mat4::from_rows(Vec4(u_row, 0),
                                       Vec4(v_row, 0),
                                       Vec4(w_row, 0),
                                       Vec4(0, 0, 0, 1)));

    ret *= transform;

    return ret;
}

inline void Texture3DCommonParams::from_params(const ConfigGroup &params)
{
    inv_u = params.child_int_or("inv_u", 0) != 0;
    inv_v = params.child_int_or("inv_v", 0) != 0;
    inv_w = params.child_int_or("inv_w", 0) != 0;
    
    if(params.find_child("uvw_perm"))
        uvw_perm = params.child_vec3i("uvw_perm");
    else
        uvw_perm = Vec3i(0, 1, 2);

    if(params.find_child("transform"))
        transform = params.child_transform3("transform");
    else
        transform = FTransform3();

    wrap_u = params.child_str_or("wrap_u", "clamp");
    wrap_v = params.child_str_or("wrap_v", "clamp");
    wrap_w = params.child_str_or("wrap_w", "clamp");

    inv_gamma = params.child_real_or("inv_gamma", 1);
}

inline real Texture3D::wrap_clamp(real x) noexcept
{
    return math::clamp<real>(x, 0, 1);
}

inline real Texture3D::wrap_repeat(real x) noexcept
{
    return math::clamp<real>(x - std::floor(x), 0, 1);
}

inline real Texture3D::wrap_mirror(real x) noexcept
{
    const int intp = static_cast<int>(x >= 0 ? x : std::floor(x));
    return math::clamp<real>(intp & 1 ? 1 - x + intp : x - intp, 0, 1);
}

inline Texture3D::WrapFuncPtr Texture3D::str_to_wrap_func(const std::string &str)
{
    if(str == "clamp")
        return  &wrap_clamp;
    if(str == "repeat")
        return &wrap_repeat;
    if(str == "mirror")
        return &wrap_mirror;
    throw ObjectConstructionException("invalid wrap function name: " + str);
}

inline void Texture3D::init_common_params(const Texture3DCommonParams &params)
{
    transform_ = params.full_transform();
    wrapper_u_ = str_to_wrap_func(params.wrap_u);
    wrapper_v_ = str_to_wrap_func(params.wrap_v);
    wrapper_w_ = str_to_wrap_func(params.wrap_w);
    inv_gamma_ = params.inv_gamma;
}

inline FSpectrum Texture3D::sample_spectrum_impl(const FVec3 &uvw) const noexcept
{
    return FSpectrum(sample_real_impl(uvw));
}

inline real Texture3D::sample_real_impl(const FVec3 &uvw) const noexcept
{
    return sample_spectrum_impl(uvw).r;
}

inline FSpectrum Texture3D::sample_spectrum(const FVec3 &uvw) const noexcept
{
    auto tuvw = transform_.apply_to_point(uvw);
    tuvw.x = wrapper_u_(tuvw.x);
    tuvw.y = wrapper_v_(tuvw.y);
    tuvw.z = wrapper_w_(tuvw.z);
    auto ret = sample_spectrum_impl(tuvw);
    if(inv_gamma_ != 1)
    {
        for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
            ret[i] = std::pow(ret[i], inv_gamma_);
    }
    return ret;
}

inline real Texture3D::sample_real(const FVec3 &uvw) const noexcept
{
    auto tuvw = transform_.apply_to_point(uvw);
    tuvw.x = wrapper_u_(tuvw.x);
    tuvw.y = wrapper_v_(tuvw.y);
    tuvw.z = wrapper_w_(tuvw.z);
    real ret = sample_real_impl(tuvw);
    if(inv_gamma_ != 1)
        ret = std::pow(ret, inv_gamma_);
    return ret;
}

AGZ_TRACER_END
