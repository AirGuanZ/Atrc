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
    const Texture *eta_i_ = nullptr;
    const Texture *eta_o_ = nullptr;

public:

    explicit DielectricFresnel(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

    static std::string description()
    {
        return R"___(
dielectric [Fresnel]
    eta_in  [Texture] inside  IOR map
    eta_out [Texture] (optional; defaultly set to 1) outside IOR map
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

        auto eta_i = TextureFactory.create(params.child_group("eta_in"), init_ctx);
        auto eta_o = defaultly_all("eta_out", 1);

        initialize(eta_o, eta_i);

        AGZ_HIERARCHY_WRAP("in initializing dielectric fresnel object")
    }

    void initialize(const Texture *eta_out, const Texture *eta_in)
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
    
Fresnel *create_dielectric_fresnel(
    const Texture *eta_out,
    const Texture *eta_in,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<DielectricFresnel>(customed_flag);
    ret->initialize(eta_out, eta_in);
    return ret;
}

AGZT_IMPLEMENTATION(Fresnel, DielectricFresnel, "dielectric")

AGZ_TRACER_END
