#include <agz/tracer/factory/creator/fresnel_creators.h>
#include <agz/tracer/factory/raw/fresnel.h>
#include <agz/tracer/factory/help.h>

AGZ_TRACER_FACTORY_BEGIN

namespace fresnel
{
    
    class AlwaysOneCreator : public Creator<Fresnel>
    {
    public:

        std::string name() const override
        {
            return "always_one";
        }

        std::shared_ptr<Fresnel> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            return create_always_one_fresnel();
        }
    };

    class DielectricCreator : public Creator<Fresnel>
    {
    public:

        std::string name() const override
        {
            return "dielectric";
        }

        std::shared_ptr<Fresnel> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto eta_in = context.create<Texture2D>(params.child_group("eta_in"));
            const auto eta_out = helper::child_texture_or_constant(context, params, "eta_out", 1);
            return create_dielectric_fresnel(std::move(eta_out), std::move(eta_in));
        }
    };

    class ConductorCreator : public Creator<Fresnel>
    {
    public:

        std::string name() const override
        {
            return "conductor";
        }

        std::shared_ptr<Fresnel> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto eta_in = context.create<Texture2D>(params.child_group("eta_in"));
            const auto eta_out = helper::child_texture_or_constant(context, params, "eta_out", 1);
            const auto k = context.create<Texture2D>(params.child_group("k"));
            return create_conductor_fresnel(std::move(eta_out), std::move(eta_in), std::move(k));
        }
    };

} // namespace fresnel

void initialize_fresnel_factory(Factory<Fresnel> &factory)
{
    factory.add_creator(std::make_unique<fresnel::AlwaysOneCreator>());
    factory.add_creator(std::make_unique<fresnel::DielectricCreator>());
    factory.add_creator(std::make_unique<fresnel::ConductorCreator>());
}

AGZ_TRACER_FACTORY_END
