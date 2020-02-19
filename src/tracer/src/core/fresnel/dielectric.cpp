#include <agz/tracer/core/fresnel.h>
#include <agz/tracer/core/texture2d.h>
#include <agz/tracer/utility/reflection.h>
#include <agz/utility/misc.h>

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
    std::shared_ptr<const Texture2D> eta_i_;
    std::shared_ptr<const Texture2D> eta_o_;

public:

    DielectricFresnel(std::shared_ptr<const Texture2D> eta_out, std::shared_ptr<const Texture2D> eta_in)
    {
        eta_i_ = eta_in;
        eta_o_ = eta_out;
    }

    FresnelPoint *get_point(const Vec2 &uv, Arena &arena) const override
    {
        const real eta_i = eta_i_->sample_real(uv);
        const real eta_o = eta_o_->sample_real(uv);
        return arena.create<DielectricFresnelPoint>(eta_i, eta_o);
    }
};

std::shared_ptr<Fresnel> create_dielectric_fresnel(
    std::shared_ptr<const Texture2D> eta_out,
    std::shared_ptr<const Texture2D> eta_in)
{
    return std::make_shared<DielectricFresnel>(eta_out, eta_in);
}

AGZ_TRACER_END
