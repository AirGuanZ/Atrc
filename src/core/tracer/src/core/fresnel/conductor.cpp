#include <agz/tracer/core/fresnel.h>
#include <agz/tracer/core/texture.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

namespace
{

    class ConductorPoint : public FresnelPoint
    {
        Spectrum eta_out_;
        Spectrum eta_in_;
        Spectrum k_;

        Spectrum eta_2_, eta_k_2_;

    public:

        ConductorPoint(const Spectrum &eta_out, const Spectrum &eta_in, const Spectrum &k) noexcept
        {
            eta_out_ = eta_out;
            eta_in_ = eta_in;
            k_ = k;

            eta_2_ = eta_in_ / eta_out_;
            eta_2_ *= eta_2_;

            eta_k_2_ = k_ / eta_out_;
            eta_k_2_ *= eta_k_2_;
        }

        Spectrum eval(real cos_theta_i) const noexcept override
        {
            if(cos_theta_i <= 0)
                return Spectrum();

            real cos2 = cos_theta_i * cos_theta_i;
            real sin2 = (std::max)(real(0), 1 - cos2);

            real(*p_sqrt)(real) = &std::sqrt;

            Spectrum t0 = eta_2_ - eta_k_2_ - sin2;
            Spectrum a2b2 = (t0 * t0 + real(4) * eta_2_ * eta_k_2_).map(p_sqrt);
            Spectrum t1 = a2b2 + cos2;
            Spectrum a = (real(0.5) * (a2b2 + t0)).map(p_sqrt);
            Spectrum t2 = 2 * cos_theta_i * a;
            Spectrum rs = (t1 - t2) / (t1 + t2);

            Spectrum t3 = cos2 * a2b2 + sin2 * sin2;
            Spectrum t4 = t2 * sin2;
            Spectrum rp = rs * (t3 - t4) / (t3 + t4);

            return real(0.5) * (rp + rs);
        }

        real eta_i() const noexcept override { return 1; }
        real eta_o() const noexcept override { return 1; }
    };

}

class Conductor : public Fresnel
{
    std::shared_ptr<const Texture> eta_out_;
    std::shared_ptr<const Texture> eta_in_;
    std::shared_ptr<const Texture> k_;

public:

    void initialize(std::shared_ptr<const Texture> eta_out, std::shared_ptr<const Texture> eta_in, std::shared_ptr<const Texture> k)
    {
        eta_out_ = eta_out;
        eta_in_ = eta_in;
        k_ = k;
    }

    FresnelPoint *get_point(const Vec2 &uv, Arena &arena) const override
    {
        Spectrum eta_out = eta_out_->sample_spectrum(uv);
        Spectrum eta_in  = eta_in_->sample_spectrum(uv);
        Spectrum k       = k_->sample_spectrum(uv);

        return arena.create<ConductorPoint>(eta_out, eta_in, k);
    }
};

std::shared_ptr<Fresnel> create_conductor_fresnel(
    std::shared_ptr<const Texture> eta_out,
    std::shared_ptr<const Texture> eta_in,
    std::shared_ptr<const Texture> k)
{
    auto ret = std::make_shared<Conductor>();
    ret->initialize(eta_out, eta_in, k);
    return ret;
}

AGZ_TRACER_END
