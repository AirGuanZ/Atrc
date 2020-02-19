#include <agz/tracer/core/medium.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class AbsorbtionMedium : public Medium
{
    Spectrum sigma_a_;

public:

    explicit AbsorbtionMedium(const Spectrum &sigma_a)
    {
        sigma_a_ = sigma_a;
    }

    int get_max_scattering_count() const noexcept override
    {
        return (std::numeric_limits<int>::max)();
    }

    Spectrum tr(const Vec3 &a, const Vec3 &b, Sampler &sampler) const noexcept override
    {
        const Spectrum exp = -sigma_a_ * (a - b).length();
        return {
            std::exp(exp.r),
            std::exp(exp.g),
            std::exp(exp.b)
        };
    }

    SampleOutScatteringResult sample_scattering(const Vec3 &a, const Vec3 &b, Sampler &sampler, Arena &arena) const override
    {
        const Spectrum tr_value = tr(a, b, sampler);
        return { { }, tr_value, nullptr };
    }
};

std::shared_ptr<Medium> create_absorbtion_medium(
    const Spectrum &sigma_a)
{
    return std::make_shared<AbsorbtionMedium>(sigma_a);
}

AGZ_TRACER_END
