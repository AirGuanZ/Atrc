#include <agz/tracer/core/fresnel.h>
#include <agz/tracer/core/texture.h>
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
    std::shared_ptr<const Texture> eta_i_;
    std::shared_ptr<const Texture> eta_o_;

public:

    void initialize(std::shared_ptr<const Texture> eta_out, std::shared_ptr<const Texture> eta_in)
    {
        eta_i_ = eta_in;
        eta_o_ = eta_out;
    }

    FresnelPoint *get_point(const Vec2 &uv, Arena &arena) const override
    {
        real eta_i = eta_i_->sample_real(uv);
        real eta_o = eta_o_->sample_real(uv);
        return arena.create<DielectricFresnelPoint>(eta_i, eta_o);
    }
};

std::shared_ptr<Fresnel> create_dielectric_fresnel(
    std::shared_ptr<const Texture> eta_out,
    std::shared_ptr<const Texture> eta_in)
{
    auto ret = std::make_shared<DielectricFresnel>();
    ret->initialize(eta_out, eta_in);
    return ret;
}

AGZ_TRACER_END
