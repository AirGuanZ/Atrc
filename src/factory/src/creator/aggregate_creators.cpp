#include <agz/factory/creator/aggregate_creators.h>
#include <agz/tracer/create/aggregate.h>

AGZ_TRACER_FACTORY_BEGIN

namespace aggregate
{

    class EntityBVHCreator : public Creator<Aggregate>
    {
    public:

        std::string name() const override
        {
            return "bvh";
        }

        std::shared_ptr<Aggregate> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const int max_leaf_size = params.child_int_or("max_leaf_size", 5);
            return create_entity_bvh(max_leaf_size);
        }
    };

    class NativeAggregateCreator : public Creator<Aggregate>
    {
    public:

        std::string name() const override
        {
            return "native";
        }

        std::shared_ptr<Aggregate> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            return create_native_aggregate();
        }
    };

} // namespace aggregate

void initialize_aggregate_factory(Factory<Aggregate> &factory)
{
    factory.add_creator(std::make_unique<aggregate::EntityBVHCreator>());
    factory.add_creator(std::make_unique<aggregate::NativeAggregateCreator>());
}

AGZ_TRACER_FACTORY_END
