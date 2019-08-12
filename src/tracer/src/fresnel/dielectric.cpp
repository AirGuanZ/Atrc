#include <agz/tracer/core/fresnel.h>
#include <agz/tracer/core/texture.h>
#include <agz/tracer/utility/reflection.h>

AGZ_TRACER_BEGIN

namespace
{
    class DielectricFresnelPoint : public FresnelPoint
    {
        real eta_i_, eta_o_;

    public:

        DielectricFresnelPoint(real eta_i, real eta_o) noexcept
            : eta_i_(eta_i), eta_o_(eta_o)
        {
            
        }

        Spectrum eval(real cos_theta_i) const noexcept override
        {
            return Spectrum(refl_aux::dielectric_fresnel(eta_i_, eta_o_, cos_theta_i));
        }

        real eta_i() const noexcept override { return eta_i_; }
        real eta_o() const noexcept override { return eta_o_; }
    };
}

class DielectricFresnel : public Fresnel
{
    Texture *eta_i_ = nullptr;
    Texture *eta_o_ = nullptr;

public:

    using Fresnel::Fresnel;

    static std::string description()
    {
        return R"___(
dielectric [Fresnel]
    eta_in  [Texture] inside  IOR map
    eta_out [Texture] outside IOR map
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        eta_i_ = TextureFactory.create(params.child_group("eta_in"), init_ctx);
        eta_o_ = TextureFactory.create(params.child_group("eta_out"), init_ctx);

        AGZ_HIERARCHY_WRAP("in initializing dielectric fresnel object")
    }

    FresnelPoint *get_point(const EntityIntersection &inct, Arena &arena) const override
    {
        real eta_i = eta_i_->sample_real(inct.uv);
        real eta_o = eta_o_->sample_real(inct.uv);
        return arena.create<DielectricFresnelPoint>(eta_i, eta_o);
    }
};

AGZT_IMPLEMENTATION(Fresnel, DielectricFresnel, "dielectric")

AGZ_TRACER_END
