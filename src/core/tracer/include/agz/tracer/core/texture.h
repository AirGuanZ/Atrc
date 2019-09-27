#pragma once

#include <agz/tracer/core/object.h>

AGZ_TRACER_BEGIN

struct TextureCommonParams
{
    bool inv_u   = false;
    bool inv_v   = false;
    bool swap_uv = false;
    Transform2 transform;

    std::string wrap_u = "clamp";
    std::string wrap_v = "clamp";

    real inv_gamma = 1;

    Transform2 full_transform() const noexcept
    {
        Transform2 ret;

        if(inv_v)
            ret *= Transform2::translate(0, 1) * Transform2::scale(1, -1);
        if(inv_u)
            ret *= Transform2::translate(1, 0) * Transform2::scale(-1, 1);
        if(swap_uv)
            ret *= Transform2(math::tmat3_c<real>(0, 1, 0, 1, 0, 0, 0, 0, 1));
        return ret * transform;
    }

    void from_params(const ConfigGroup &params)
    {
        inv_u = params.child_int_or("inv_u", 0) != 0;
        inv_v = params.child_int_or("inv_v", 0) != 0;
        swap_uv = params.child_int_or("swap_uv", 0) != 0;

        if(params.find_child("transform"))
            transform = params.child_transform2("transform");
        else
            transform = Transform2();

        wrap_u = params.child_str_or("wrap_u", "clamp");
        wrap_v = params.child_str_or("wrap_v", "clamp");

        inv_gamma = params.child_real_or("inv_gamma", 1);
    }
};

/**
 * @brief 纹理接口
 * 
 * sample_spectrum和sample_real中至少要实现一个
 */
class Texture : public obj::Object
{
    Transform2 transform_;

    using WrapFunctionPtr = real(*)(real);
    WrapFunctionPtr wrapper_u_ = &wrap_clamp;
    WrapFunctionPtr wrapper_v_ = &wrap_clamp;

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
        int intp = static_cast<int>(x >= 0 ? x : std::floor(x));
        return math::clamp<real>(intp & 1 ? 1 - x + intp : x - intp, 0, 1);
    }

protected:

    void init_common_params(const TextureCommonParams &params)
    {
        transform_ = params.full_transform();

        if(params.wrap_u == "clamp")
            wrapper_u_ = &wrap_clamp;
        else if(params.wrap_u == "repeat")
            wrapper_u_ = &wrap_repeat;
        else if(params.wrap_u == "mirror")
            wrapper_u_ = &wrap_mirror;
        else
            throw ObjectConstructionException("invalid wrap_u value: " + params.wrap_u + " (expect clamp/repeat/mirror)");

        if(params.wrap_v == "clamp")
            wrapper_v_ = &wrap_clamp;
        else if(params.wrap_v == "repeat")
            wrapper_v_ = &wrap_repeat;
        else if(params.wrap_v == "mirror")
            wrapper_v_ = &wrap_mirror;
        else
            throw ObjectConstructionException("invalid wrap_v value: " + params.wrap_v + " (expect clamp/repeat/mirror)");

        inv_gamma_ = params.inv_gamma;
    }

    /**
     * @brief 初始化纹理坐标变换和gamma校正
     */
    void init_common_params(const Config &params)
    {
        /*if(auto node = params.find_child("inv_v"); node && node->as_value().as_int())
            transform_ *= Transform2::translate(0, 1) * Transform2::scale(1, -1);
        if(auto node = params.find_child("inv_u"); node && node->as_value().as_int())
            transform_ *= Transform2::translate(1, 0) * Transform2::scale(-1, 1);
        if(auto node = params.find_child("swap_uv"); node && node->as_value().as_int())
            transform_ *= Transform2(math::tmat3_c<real>(0, 1, 0, 1, 0, 0, 0, 0, 1));
        if(params.find_child("transform"))
            transform_ *= params.child_transform2("transform");

        if(auto node = params.find_child("wrap_u"))
        {
            auto &wrapper = node->as_value().as_str();
            if(wrapper == "clamp")
                wrapper_u_ = &wrap_clamp;
            else if(wrapper == "repeat")
                wrapper_u_ = &wrap_repeat;
            else if(wrapper == "mirror")
                wrapper_u_ = &wrap_mirror;
            else
                throw ObjectConstructionException("invalid wrap_u value: " + wrapper + " (expect clamp/repeat/mirror)");
        }

        if(auto node = params.find_child("wrap_v"))
        {
            auto &wrapper = node->as_value().as_str();
            if(wrapper == "clamp")
                wrapper_v_ = &wrap_clamp;
            else if(wrapper == "repeat")
                wrapper_v_ = &wrap_repeat;
            else if(wrapper == "mirror")
                wrapper_v_ = &wrap_mirror;
            else
                throw ObjectConstructionException("invalid wrap_v value: " + wrapper + " (expect clamp/repeat/mirror)");
        }

        if(auto node = params.find_child("inv_gamma"))
            inv_gamma_ = node->as_value().as_real();*/

        TextureCommonParams common_params;
        common_params.from_params(params);
        init_common_params(common_params);
    }

    virtual Spectrum sample_spectrum_impl(const Vec2 &uv) const noexcept
    {
        return Spectrum(sample_real_impl(uv));
    }

    virtual real sample_real_impl(const Vec2 &uv) const noexcept
    {
        return sample_spectrum_impl(uv).r;
    }

public:

    using Object::Object;

    /**
     * @brief 采样特定uv点处的Spectrum
     */
    virtual Spectrum sample_spectrum(const Vec2 &uv) const noexcept
    {
        Vec2 uv1 = transform_.apply_to_point(uv);
        real u = wrapper_u_(uv1.x);
        real v = wrapper_v_(uv1.y);
        auto ret = sample_spectrum_impl({ u, v });
        if(inv_gamma_ != 1)
        {
            for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
                ret[i] = std::pow(ret[i], inv_gamma_);
        }
        return ret;
    }

    /**
     * @brief 采样特定uv点处的实数值
     */
    virtual real sample_real(const Vec2 &uv) const noexcept
    {
        Vec2 uv1 = transform_.apply_to_point(uv);
        real u = wrapper_u_(uv1.x);
        real v = wrapper_v_(uv1.y);
        auto ret = sample_real_impl({ u, v });
        if(inv_gamma_ != 1)
            ret = std::pow(ret, inv_gamma_);
        return ret;
    }

    /**
     * @brief 纹理宽度
     */
    virtual int width() const noexcept = 0;

    /**
     * @brief 纹理高度
     */
    virtual int height() const noexcept = 0;
};

AGZT_INTERFACE(Texture)

AGZ_TRACER_END
