#include <agz/tracer/core/medium.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class VoidMedium : public Medium
{
public:

    int get_max_scattering_count() const noexcept override
    {
        return (std::numeric_limits<int>::max)();
    }

    Spectrum tr(const Vec3 &a, const Vec3 &b, Sampler &sampler) const noexcept override
    {
        return Spectrum(1);
    }

    SampleOutScatteringResult sample_scattering(const Vec3 &a, const Vec3 &b, Sampler &sampler, Arena &arena) const override
    {
        return { { }, Spectrum(1), nullptr };
    }
};

std::shared_ptr<Medium> create_void()
{
    return std::make_shared<VoidMedium>();
}

AGZ_TRACER_END
