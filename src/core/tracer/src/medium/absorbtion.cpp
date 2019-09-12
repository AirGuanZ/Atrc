#include <agz/tracer/core/medium.h>

AGZ_TRACER_BEGIN

class AbsorbtionMedium : public Medium
{
    Spectrum sigma_a_; // 吸收系数

public:

    static std::string description()
    {
        return R"___(
absorb [Medium]
    sigma_a [Spectrum] absorbtion index
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext&) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        sigma_a_ = params.child_spectrum("sigma_a");

        AGZ_HIERARCHY_WRAP("in initializing absorbtion medium")
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

AGZT_IMPLEMENTATION(Medium, AbsorbtionMedium, "absorb")

AGZ_TRACER_END
