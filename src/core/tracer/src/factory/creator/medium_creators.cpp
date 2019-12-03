#include <agz/tracer/factory/creator/medium_creators.h>
#include <agz/tracer/factory/raw/medium.h>

AGZ_TRACER_FACTORY_BEGIN

namespace medium
{
    
    class AbsorbtionMediumCreator : public Creator<Medium>
    {
    public:

        std::string name() const override
        {
            return "absorb";
        }

        std::shared_ptr<Medium> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto sigma_a = params.child_spectrum("sigma_a");
            return create_absorbtion_medium(sigma_a);
        }
    };

    class HeterogeneousMediumCreator : public Creator<Medium>
    {
    public:

        std::string name() const override
        {
            return "heterogeneous";
        }

        std::shared_ptr<Medium> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto local_to_world = params.child_transform3("transform");
            auto density = context.create<Texture3D>(params.child_group("density"));
            auto albedo  = context.create<Texture3D>(params.child_group("albedo"));
            auto g       = context.create<Texture3D>(params.child_group("g"));
            return create_heterogeneous_medium(local_to_world, std::move(density), std::move(albedo), std::move(g));
        }
    };

    class HomogeneousMediumCreator : public Creator<Medium>
    {
    public:

        std::string name() const override
        {
            return "homogeneous";
        }

        std::shared_ptr<Medium> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto sigma_a = params.child_spectrum("sigma_a");
            auto sigma_s = params.child_spectrum("sigma_s");
            real g = params.child_real("g");
            return create_homogeneous_medium(sigma_a, sigma_s, g);
        }
    };

    class VoidMediumCreator : public Creator<Medium>
    {
    public:

        std::string name() const override
        {
            return "void";
        }

        std::shared_ptr<Medium> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            return create_void();
        }
    };

} // namespace medium

void initialize_medium_factory(Factory<Medium> &factory)
{
    factory.add_creator(std::make_unique<medium::AbsorbtionMediumCreator>());
    factory.add_creator(std::make_unique<medium::HeterogeneousMediumCreator>());
    factory.add_creator(std::make_unique<medium::HomogeneousMediumCreator>());
    factory.add_creator(std::make_unique<medium::VoidMediumCreator>());
}

AGZ_TRACER_FACTORY_END
