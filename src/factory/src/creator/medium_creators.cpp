#include <agz/factory/creator/medium_creators.h>
#include <agz/tracer/create/medium.h>

AGZ_TRACER_FACTORY_BEGIN

namespace medium
{

    class HeterogeneousMediumCreator : public Creator<Medium>
    {
    public:

        std::string name() const override
        {
            return "heterogeneous";
        }

        RC<Medium> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto local_to_world = params.child_transform3("transform");
            const auto density = context.create<Texture3D>(
                params.child_group("density"));
            const auto albedo  = context.create<Texture3D>(
                params.child_group("albedo"));
            const auto g       = context.create<Texture3D>(
                params.child_group("g"));

            const int max_scat_count = params.child_int_or(
                "max_scattering_count", (std::numeric_limits<int>::max)());
            const bool white_for_indirect =
                params.child_int_or("white_for_indirect", 0) != 0;

            return create_heterogeneous_medium(
                local_to_world, std::move(density),
                std::move(albedo), std::move(g),
                max_scat_count, white_for_indirect);
        }
    };

    class HomogeneousMediumCreator : public Creator<Medium>
    {
    public:

        std::string name() const override
        {
            return "homogeneous";
        }

        RC<Medium> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto sigma_a = params.child_spectrum("sigma_a");
            const auto sigma_s = params.child_spectrum("sigma_s");
            const real g = params.child_real("g");

            const int max_scat_count = params.child_int_or(
                "max_scattering_count", (std::numeric_limits<int>::max)());
            
            return create_homogeneous_medium(
                sigma_a, sigma_s, g, max_scat_count);
        }
    };

    class VoidMediumCreator : public Creator<Medium>
    {
    public:

        std::string name() const override
        {
            return "void";
        }

        RC<Medium> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            return create_void();
        }
    };

} // namespace medium

void initialize_medium_factory(Factory<Medium> &factory)
{
    factory.add_creator(newBox<medium::HeterogeneousMediumCreator>());
    factory.add_creator(newBox<medium::HomogeneousMediumCreator>());
    factory.add_creator(newBox<medium::VoidMediumCreator>());
}

AGZ_TRACER_FACTORY_END
