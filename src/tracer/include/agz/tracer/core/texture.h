#pragma once

#include <agz/tracer_utility/object.h>

AGZ_TRACER_BEGIN

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
        return x;
    }

    static real wrap_repeat(real x) noexcept
    {
        return x - std::floor(x);
    }

    static real wrap_mirror(real x) noexcept
    {
        int intp = static_cast<int>(x >= 0 ? x : std::floor(x));
        return intp & 1 ? 1 - x + intp : x - intp;
    }

protected:

    void init_transform(const Config &params)
    {
        if(auto node = params.find_child("inv_v"); node && node->as_value().as_int())
            transform_ *= Transform2::translate(0, 1) * Transform2::scale(1, -1) * transform_;
        if(auto node = params.find_child("inv_u"); node && node->as_value().as_int())
            transform_ *= Transform2::translate(1, 0) * Transform2::scale(-1, 1) * transform_;
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
            inv_gamma_ = node->as_value().as_real();
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
        real u = math::clamp<real>(wrapper_u_(uv1.x), 0, 1);
        real v = math::clamp<real>(wrapper_v_(uv1.y), 0, 1);
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
        real u = math::clamp<real>(wrapper_u_(uv1.x), 0, 1);
        real v = math::clamp<real>(wrapper_v_(uv1.y), 0, 1);
        auto ret = sample_real_impl({ u, v });
        if(inv_gamma_ != 1)
            ret = std::pow(ret, inv_gamma_);
        return ret;
    }
};

AGZT_INTERFACE(Texture)

AGZ_TRACER_END
