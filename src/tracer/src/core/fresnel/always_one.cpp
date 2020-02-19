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

    FresnelPoint *get_point(const Vec2 &uv, Arena &arena) const override
    {
        return arena.create<AlwaysOnePoint>();
    }
};

std::shared_ptr<Fresnel> create_always_one_fresnel()
{
    return std::make_shared<AlwaysOne>();
}

AGZ_TRACER_END
