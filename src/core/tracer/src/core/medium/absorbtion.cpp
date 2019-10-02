#include <agz/tracer/core/medium.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class AbsorbtionMedium : public Medium
{
    Spectrum sigma_a_; // 吸收系数

public:

    void initialize(const Spectrum &sigma_a)
    {
        sigma_a_ = sigma_a;
    }

    Spectrum tr(const Vec3 &a, const Vec3 &b) const noexcept override
    {
        Spectrum exp = -sigma_a_ * (a - b).length();
        return {
            std::exp(exp.r),
            std::exp(exp.g),
            std::exp(exp.b)
        };
    }

    SampleMediumResult sample(const Vec3 &o, const Vec3 &d, real t_min, real t_max, const Sample1 &sam) const noexcept override
    {
        return SAMPLE_MEDIUM_RESULT_INVALID;
    }

    ShadingPoint shade(const MediumIntersection &inct, Arena &arena) const noexcept override
    {
        return { nullptr };
    }
};

std::shared_ptr<Medium> create_absorbtion_medium(
    const Spectrum &sigma_a)
{
    auto ret = std::make_shared<AbsorbtionMedium>();
    ret->initialize(sigma_a);
    return ret;
}

AGZ_TRACER_END
