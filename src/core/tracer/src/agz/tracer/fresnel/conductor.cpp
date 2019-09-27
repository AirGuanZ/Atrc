#include <agz/tracer/core/fresnel.h>
#include <agz/tracer/core/texture.h>

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
    const Texture *eta_out_ = nullptr;
    const Texture *eta_in_  = nullptr;
    const Texture *k_       = nullptr;

public:

    explicit Conductor(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

    static std::string description()
    {
        return R"___(
conductor [Fresnel]
    eta_out [Texture] (optional; defaultly set to 1) outside IOR map
    eta_in  [Texture] inside  IOR map
    k       [Texture] absorptivity map
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        auto defaultly_all = [&](const char *name, real default_value)
        {
            if(auto node = params.find_child_group(name))
                return TextureFactory.create(*node, init_ctx);

            ConfigGroup group;
            group.insert_child("type", std::make_shared<ConfigValue>("constant"));

            auto arr = std::make_shared<ConfigArray>();
            arr->push_back(std::make_shared<ConfigValue>(std::to_string(default_value)));
            group.insert_child("texel", std::move(arr));

            return TextureFactory.create(group, init_ctx);
        };

        auto eta_out = defaultly_all("eta_out", 1);
        auto eta_in  = TextureFactory.create(params.child_group("eta_in"), init_ctx);
        auto k       = TextureFactory.create(params.child_group("k"), init_ctx);

        initialize(eta_out, eta_in, k);

        AGZ_HIERARCHY_WRAP("in initializing conductor fresnel object")
    }

    void initialize(const Texture *eta_out, const Texture *eta_in, const Texture *k)
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

Fresnel *create_conductor_fresnel(
    const Texture *eta_out,
    const Texture *eta_in,
    const Texture *k,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<Conductor>(customed_flag);
    ret->initialize(eta_out, eta_in, k);
    return ret;
}

AGZT_IMPLEMENTATION(Fresnel, Conductor, "conductor")

AGZ_TRACER_END
