#include <agz/tracer/core/fresnel.h>

AGZ_TRACER_BEGIN

namespace
{
    
    class AlwaysOnePoint : public FresnelPoint
    {
    public:

        Spectrum eval(real cos_theta_i) const noexcept override
        {
            return Spectrum(1);
        }

        real eta_i() const noexcept override { return 1; }
        real eta_o() const noexcept override { return 1; }
    };

} // namespace anonymous

class AlwaysOne : public Fresnel
{
public:

    using Fresnel::Fresnel;

    static std::string description()
    {
        return "always_one [Fresnel]";
    }

    FresnelPoint *get_point(const Vec2 &uv, Arena &arena) const override
    {
        return arena.create<AlwaysOnePoint>();
    }
};

AGZT_IMPLEMENTATION(Fresnel, AlwaysOne, "always_one")

AGZ_TRACER_END
