#pragma once

#include <agz/tracer/core/bsdf.h>

AGZ_TRACER_BEGIN

class BSDFComponent : public misc::uncopyable_t
{
    uint8_t type_;

public:

    struct SampleResult
    {
        Vec3 lwi;
        Spectrum f;
        real pdf = 0;

        bool is_valid() const noexcept
        {
            return pdf != 0;
        }
    };

    explicit BSDFComponent(uint8_t type) noexcept;

    virtual ~BSDFComponent() = default;

    bool is_contained_in(uint8_t type) const noexcept;

    uint8_t get_component_type() const noexcept;

    virtual Spectrum eval(
        const Vec3 &lwi, const Vec3 &lwo, TransMode mode) const noexcept = 0;

    virtual real pdf(const Vec3 &lwi, const Vec3 &lwo) const noexcept = 0;

    virtual SampleResult sample(
        const Vec3 &lwo, TransMode mode, const Sample2 &sam) const noexcept = 0;
};

inline BSDFComponent::BSDFComponent(uint8_t type) noexcept
    : type_(type)
{

}

inline bool BSDFComponent::is_contained_in(uint8_t type) const noexcept
{
    return (type_ & type) == type_;
}

inline uint8_t BSDFComponent::get_component_type() const noexcept
{
    return type_;
}

AGZ_TRACER_END
