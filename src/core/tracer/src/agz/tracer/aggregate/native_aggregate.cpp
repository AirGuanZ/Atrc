#include <agz/tracer/core/aggregate.h>
#include <agz/tracer/core/entity.h>

AGZ_TRACER_BEGIN

class NativeAggregate : public Aggregate
{
    std::vector<const Entity*> entities_;
    AABB world_bound_;

public:

    explicit NativeAggregate(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

    static std::string description()
    {
        return "native [Aggregate]";
    }

    void initialize(const Config &params, obj::ObjectInitContext&) override
    {
        init_customed_flag(params);
    }

    void build(const Entity *const*entities, size_t entity_count) override
    {
        entities_ = std::vector<const Entity*>(entities, entities + entity_count);
        for(size_t i = 0; i < entity_count; ++i)
            world_bound_ |= entities[i]->world_bound();
    }

    bool has_intersection(const Ray &r) const noexcept override
    {
        for(auto ent : entities_)
        {
            if(ent->has_intersection(r))
                return true;
        }
        return false;
    }

    bool closest_intersection(const Ray &r, EntityIntersection *inct) const noexcept override
    {
        Ray ray = r;
        bool ret = false;
        for(auto ent : entities_)
        {
            if(ent->closest_intersection(ray, inct))
            {
                ray.t_max = inct->t;
                ret = true;
            }
        }
        return ret;
    }

    AABB world_bound() const noexcept override
    {
        return world_bound_;
    }
};

Aggregate *create_native_aggregate(
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<NativeAggregate>(customed_flag);
    return ret;
}

AGZT_IMPLEMENTATION(Aggregate, NativeAggregate, "native")

AGZ_TRACER_END
