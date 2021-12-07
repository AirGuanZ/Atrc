#include <agz/tracer/core/medium.h>
#include <agz-utils/misc.h>

AGZ_TRACER_BEGIN

class VoidMedium : public Medium
{
public:

    int get_max_scattering_count() const noexcept override
    {
        return (std::numeric_limits<int>::max)();
    }

    FSpectrum tr(
        const FVec3 &a, const FVec3 &b, Sampler &sampler) const noexcept override
    {
        return FSpectrum(1);
    }

    FSpectrum ab(
        const FVec3 &a, const FVec3 &b, Sampler &sampler) const noexcept override
    {
        return FSpectrum(1);
    }

    SampleOutScatteringResult sample_scattering(
        const FVec3 &a, const FVec3 &b,
        Sampler &sampler, Arena &arena, bool) const override
    {
        return { { }, FSpectrum(1), nullptr };
    }
};

RC<Medium> create_void()
{
    static RC<Medium> ret = newRC<VoidMedium>();
    return ret;
}

AGZ_TRACER_END
