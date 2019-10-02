#include <agz/tracer/core/aggregate.h>
#include <agz/tracer/core/entity.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

namespace
{
    
    struct Leaf;
    struct Interior;

    using Node = misc::variant_t<Leaf, Interior>;

    struct Leaf
    {
        AABB bound;
        size_t start = 0, end = 0;
    };

    struct Interior
    {
        AABB bound;
        size_t left  = 0, right = 0;
    };

    struct EntityRecord
    {
        const Entity *entity = nullptr;
        AABB bound;
    };

} // namespace anonymous

class EntityBVH : public Aggregate
{
    using EntityPtr = const Entity*;

    std::vector<Node> nodes_;
    std::vector<EntityPtr> prims_;

    std::vector<std::shared_ptr<const Entity>> entities_;
    
    int max_leaf_size_ = 5;

    size_t build_aux(EntityRecord *entities, size_t count)
    {
        assert(count);

        if(count <= static_cast<size_t>(max_leaf_size_) || count < 2)
        {
            size_t start = prims_.size();
            size_t end   = start + count;
            
            AABB bound;
            for(size_t i = 0; i < count; ++i)
            {
                prims_.push_back(entities[i].entity);
                bound |= entities[i].bound;
            }

            size_t ret = nodes_.size();
            nodes_.emplace_back(Leaf{ bound, start, end });
            return ret;
        }

        // find splitting axis

        AABB all_bound;
        for(size_t i = 0; i < count; ++i)
            all_bound |= entities[i].bound;
        real split_axis_len = -1;
        int split_axis = 0;
        for(int i = 0; i < 3; ++i)
        {
            real axis_len = all_bound.high[i] - all_bound.low[i];
            if(axis_len > split_axis_len)
            {
                split_axis_len = axis_len;
                split_axis = i;
            }
        }

        // sort entities according to centre[split_axis]

        std::sort(entities, entities + count,
            [split_axis](const EntityRecord &lhs, const EntityRecord &rhs)
        {
            real L = lhs.bound.low[split_axis] + lhs.bound.high[split_axis];
            real R = rhs.bound.low[split_axis] + rhs.bound.high[split_axis];
            return L < R;
        });

        // push back new interior node

        size_t interior_idx = nodes_.size();
        nodes_.emplace_back(Interior());

        // build left & right children

        size_t split_idx = count / 2;
        size_t left_idx  = build_aux(entities, split_idx);
        size_t right_idx = build_aux(entities + split_idx, count - split_idx);

        // fill interior node

        auto &interior = nodes_[interior_idx].as<Interior>();
        interior.bound = all_bound;
        interior.left  = left_idx;
        interior.right = right_idx;

        return interior_idx;
    }

    bool has_intersection_aux(const Vec3 &inv_dir, const Ray &r, const Node &node) const
    {
        return match_variant(node,
            [&](const Leaf &leaf)
        {
            if(!leaf.bound.intersect(r.o, inv_dir, r.t_min, r.t_max))
                return false;
            for(size_t i = leaf.start; i < leaf.end; ++i)
            {
                if(prims_[i]->has_intersection(r))
                    return true;
            }
            return false;
        },
            [&](const Interior &interior)
        {
            if(!interior.bound.intersect(r.o, inv_dir, r.t_min, r.t_max))
                return false;
            return has_intersection_aux(inv_dir, r, nodes_[interior.left]) ||
                   has_intersection_aux(inv_dir, r, nodes_[interior.right]);
        });
    }

    bool closest_intersection_aux(const Vec3 &inv_dir, Ray &r, const Node &node, EntityIntersection *inct) const
    {
        return match_variant(node,
            [&](const Leaf &leaf)
        {
            if(!leaf.bound.intersect(r.o, inv_dir, r.t_min, r.t_max))
                return false;
            bool ret = false;
            for(size_t i = leaf.start; i < leaf.end; ++i)
            {
                auto ent = prims_[i];
                if(ent->closest_intersection(r, inct))
                {
                    r.t_max = inct->t;
                    ret = true;
                }
            }
            return ret;
        },
            [&](const Interior &interior)
        {
            if(!interior.bound.intersect(r.o, inv_dir, r.t_min, r.t_max))
                return false;

            bool left  = closest_intersection_aux(inv_dir, r, nodes_[interior.left], inct);
            bool right = closest_intersection_aux(inv_dir, r, nodes_[interior.right], inct);

            return left || right;
        });
    }

public:

    void initialize(int max_leaf_size)
    {
        AGZ_HIERARCHY_TRY

        max_leaf_size_ = max_leaf_size;
        if(max_leaf_size < 1)
            throw ObjectConstructionException("invalid max_leaf_size value");
      
        AGZ_HIERARCHY_WRAP("in initializing entity bvh aggregate")
    }

    void build(const std::vector<std::shared_ptr<const Entity>> &entities) override
    {
        if(entities.empty())
        {
            nodes_.emplace_back(Leaf{ AABB{ Vec3(0), Vec3(1) }, 0, 0 });
            return;
        }

        std::vector<EntityRecord> records(entities.size());
        prims_.reserve(entities.size());
        for(size_t i = 0; i < entities.size(); ++i)
            records[i] = { entities[i].get(), entities[i]->world_bound() };

        entities_ = entities;
        build_aux(records.data(), records.size());
    }

    bool has_intersection(const Ray &r) const noexcept override
    {
        Vec3 inv_dir(1 / r.d.x, 1 / r.d.y, 1 / r.d.z);
        return has_intersection_aux(inv_dir, r, nodes_[0]);
    }

    bool closest_intersection(const Ray &r, EntityIntersection *inct) const noexcept override
    {
        Vec3 inv_dir(1 / r.d.x, 1 / r.d.y, 1 / r.d.z);
        Ray ray = r;
        return closest_intersection_aux(inv_dir, ray, nodes_[0], inct);
    }

    AABB world_bound() const noexcept override
    {
        return match_variant(nodes_[0],
            [](const Leaf &leaf)
        {
            return leaf.bound;
        },
            [](const Interior &interior)
        {
            return interior.bound;
        });
    }
};

std::shared_ptr<Aggregate> create_entity_bvh(
    int max_leaf_size)
{
    auto ret = std::make_shared<EntityBVH>();
    ret->initialize(max_leaf_size);
    return ret;
}

AGZ_TRACER_END
