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
    Transform3 transform;

    std::string wrap_u = "clamp";
    std::string wrap_v = "clamp";
    std::string wrap_w = "clamp";

    real inv_gamma = 1;

    Transform3 full_transform() const
    {
        using Trans4 = Mat4::left_transform;

        Transform3 ret;
        
        if(inv_u)
            ret *= Transform3(Trans4::translate(1, 0, 0) * Trans4::scale(-1, 1, 1));
        if(inv_v)
            ret *= Transform3(Trans4::translate(0, 1, 0) * Trans4::scale(1, -1, 1));
        if(inv_w)
            ret *= Transform3(Trans4::translate(0, 0, 1) * Trans4::scale(1, 1, -1));

        auto perm_to_row = [&](int perm)
        {
            if(perm < 0 || perm >= 3)
                throw ObjectConstructionException("invalid uvw perm value");
            Vec3 row;
            row[perm] = 1;
            return row;
        };
        const Vec3 u_row = perm_to_row(uvw_perm.x);
        const Vec3 v_row = perm_to_row(uvw_perm.y);
        const Vec3 w_row = perm_to_row(uvw_perm.z);

        ret *= Transform3(Mat4::from_rows(Vec4(u_row,          0),
                                            Vec4(v_row,          0),
                                            Vec4(w_row,          0),
                                            Vec4(0, 0, 0, 1)));

        ret *= transform;

        return ret;
    }

    void from_params(const ConfigGroup &params)
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
            transform = Transform3();

        wrap_u = params.child_str_or("wrap_u", "clamp");
        wrap_v = params.child_str_or("wrap_v", "clamp");
        wrap_w = params.child_str_or("wrap_w", "clamp");

        inv_gamma = params.child_real_or("inv_gamma", 1);
    }
};

/**
 * @brief 3d texture interface
 */
class Texture3D
{
protected:

    Transform3 transform_;

    using WrapFuncPtr = real(*)(real);

    WrapFuncPtr wrapper_u_ = &wrap_clamp;
    WrapFuncPtr wrapper_v_ = &wrap_clamp;
    WrapFuncPtr wrapper_w_ = &wrap_clamp;

    real inv_gamma_ = 1;

    static real wrap_clamp(real x) noexcept
    {
        return math::clamp<real>(x, 0, 1);
    }

    static real wrap_repeat(real x) noexcept
    {
        return math::clamp<real>(x - std::floor(x), 0, 1);
    }

    static real wrap_mirror(real x) noexcept
    {
        const int intp = static_cast<int>(x >= 0 ? x : std::floor(x));
        return math::clamp<real>(intp & 1 ? 1 - x + intp : x - intp, 0, 1);
    }

    static WrapFuncPtr str_to_wrap_func(const std::string &str)
    {
        if(str == "clamp")
            return  &wrap_clamp;
        if(str == "repeat")
            return &wrap_repeat;
        if(str == "mirror")
            return &wrap_mirror;
        throw ObjectConstructionException("invalid wrap function name: " + str);
    }

    void init_common_params(const Texture3DCommonParams &params)
    {
        transform_ = params.full_transform();
        wrapper_u_ = str_to_wrap_func(params.wrap_u);
        wrapper_v_ = str_to_wrap_func(params.wrap_v);
        wrapper_w_ = str_to_wrap_func(params.wrap_w);
        inv_gamma_ = params.inv_gamma;
    }

    virtual Spectrum sample_spectrum_impl(const Vec3 &uvw) const noexcept
    {
        return Spectrum(sample_real_impl(uvw));
    }

    virtual real sample_real_impl(const Vec3 &uvw) const noexcept
    {
        return sample_spectrum_impl(uvw).r;
    }

public:

    virtual ~Texture3D() = default;

    /**
     * @brief sample spectrum value at uv
     */
    virtual Spectrum sample_spectrum(const Vec3 &uvw) const noexcept
    {
        auto [u, v, w] = transform_.apply_to_point(uvw);
        u = wrapper_u_(u);
        v = wrapper_v_(v);
        w = wrapper_w_(w);
        auto ret = sample_spectrum_impl({ u, v , w });
        if(inv_gamma_ != 1)
        {
            for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
                ret[i] = std::pow(ret[i], inv_gamma_);
        }
        return ret;
    }
    
    /**
     * @brief sample real value at uv
     */
    virtual real sample_real(const Vec3 &uvw) const noexcept
    {
        auto [u, v, w] = transform_.apply_to_point(uvw);
        u = wrapper_u_(u);
        v = wrapper_v_(v);
        w = wrapper_w_(w);
        real ret = sample_real_impl({ u, v , w });
        if(inv_gamma_ != 1)
            ret = std::pow(ret, inv_gamma_);
        return ret;
    }

    virtual int width() const noexcept = 0;

    virtual int height() const noexcept = 0;

    virtual int depth() const noexcept = 0;

    /**
     * @brief element-wise maximal spectrum value
     */
    virtual Spectrum max_spectrum() const noexcept = 0;

    /**
     * @brief element-wise minimal spectrum value
     */
    virtual Spectrum min_spectrum() const noexcept = 0;

    /**
     * @brief maximal real value
     */
    virtual real max_real() const noexcept = 0;

    /**
     * @brief minimal real value
     */
    virtual real min_real() const noexcept = 0;
};

AGZ_TRACER_END
