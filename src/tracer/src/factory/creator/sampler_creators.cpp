#include <agz/tracer/factory/creator/sampler_creators.h>
#include <agz/tracer/factory/raw/sampler.h>

AGZ_TRACER_FACTORY_BEGIN

namespace sampler
{
    
    class NativeSamplerCreator : public Creator<Sampler>
    {
    public:

        std::string name() const override
        {
            return "native";
        }

        std::shared_ptr<Sampler> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto seed_node = params.find_child_value("seed");
            const bool use_seed = seed_node != nullptr;

            const int spp = params.child_int("spp");

            return create_native_sampler(spp, use_seed ? seed_node->as_int() : 0, !use_seed);
        }
    };

    class SobolSamplerCreator : public Creator<Sampler>
    {
    public:

        std::string name() const override
        {
            return "sobol";
        }

        std::shared_ptr<Sampler> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto seed_node = params.find_child_value("seed");
            const bool use_seed = seed_node != nullptr;

            const int spp = params.child_int("spp");

            return create_sobol_sampler(spp, use_seed ? seed_node->as_int() : 0, !use_seed);
        }
    };

} // namespace sampler

void initialize_sampler_factory(Factory<Sampler> &factory)
{
    factory.add_creator(std::make_unique<sampler::NativeSamplerCreator>());
    factory.add_creator(std::make_unique<sampler::SobolSamplerCreator>());
}

AGZ_TRACER_FACTORY_END
