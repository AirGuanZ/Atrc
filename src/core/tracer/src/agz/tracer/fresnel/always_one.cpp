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

    explicit AlwaysOne(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

    static std::string description()
    {
        return "always_one [Fresnel]";
    }

    FresnelPoint *get_point(const Vec2 &uv, Arena &arena) const override
    {
        return arena.create<AlwaysOnePoint>();
    }
};

Fresnel *create_always_one_fresnel(
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<AlwaysOne>(customed_flag);
    return ret;
}

AGZT_IMPLEMENTATION(Fresnel, AlwaysOne, "always_one")

AGZ_TRACER_END
