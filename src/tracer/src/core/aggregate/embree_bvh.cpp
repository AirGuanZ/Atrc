#ifdef USE_EMBREE

#include <embree3/rtcore.h>

#include <agz/tracer/core/aggregate.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/utility/embree.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

namespace
{

    struct Leaf;
    struct Interior;

    struct Node;

    struct Interior
    {
        Node *left, *right;
        AABB left_bound, right_bound;
    };

    struct Leaf
    {
        size_t start, end;
    };

    struct Node
    {
        bool is_interior = true;

        union
        {
            Interior interior;
            Leaf leaf;
        };
    };

} // namespace anonymous

class EntityBVHEmbree : public Aggregate
{
    using EntityPtr = const Entity *;

    int max_leaf_size_;

    std::vector<EntityPtr> prims_;

    std::vector<RC<const Entity>> entities_;

    RTCBVH bvh_;
    Node *root_;

    struct BuildingParams
    {
        std::vector<EntityPtr>        &output_prims;
        std::vector<RC<const Entity>> &all_entities;
    };

    static AABB rtc_bound_to_aabb(const RTCBounds &bbox) noexcept
    {
        return {
            { bbox.lower_x, bbox.lower_y, bbox.lower_z },
            { bbox.upper_x, bbox.upper_y, bbox.upper_z }
        };
    }

    static void *create_interior(
        RTCThreadLocalAllocator alloc,
        unsigned int num_child, void *user_ptr)
    {
        assert(num_child == 2);

        Node *node = static_cast<Node*>(rtcThreadLocalAlloc(
            alloc, sizeof(Node), alignof(Node)));

        node->is_interior          = true;
        node->interior.left        = nullptr;
        node->interior.right       = nullptr;
        node->interior.left_bound  = {};
        node->interior.right_bound = {};

        return node;
    }

    static void set_interior_children(
        void *raw_node, void **children,
        unsigned int num_child, void *user_ptr)
    {
        assert(num_child == 2);
        auto node = static_cast<Node *>(raw_node);
        node->interior.left  = static_cast<Node *>(children[0]);
        node->interior.right = static_cast<Node *>(children[1]);
    }

    static void set_interior_bounds(
        void *raw_node, const RTCBounds **bounds,
        unsigned int num_child, void *user_ptr)
    {
        assert(num_child == 2);
        auto node = static_cast<Node *>(raw_node);
        node->interior.left_bound  = rtc_bound_to_aabb(*bounds[0]);
        node->interior.right_bound = rtc_bound_to_aabb(*bounds[1]);
    }

    static void *create_leaf(
        RTCThreadLocalAllocator alloc,
        const RTCBuildPrimitive *prims, size_t num_prims,
        void *user_ptr)
    {
        Node *node = static_cast<Node *>(rtcThreadLocalAlloc(
            alloc, sizeof(Node), alignof(Node)));
        node->is_interior = false;

        auto &params = *static_cast<BuildingParams *>(user_ptr);

        const size_t start = params.output_prims.size();
        for(size_t i = 0; i < num_prims; ++i)
        {
            params.output_prims.push_back(
                params.all_entities[prims[i].primID].get());
        }
        const size_t end = params.output_prims.size();

        node->leaf.start = start;
        node->leaf.end   = end;

        return node;
    }

    bool has_intersection_aux(
        const FVec3 &inv_dir, const Ray &r, const Node &node) const noexcept
    {
        if(node.is_interior)
        {
            const bool inct_left = node.interior.left_bound.intersect(
                r.o, inv_dir, r.t_min, r.t_max);
            if(inct_left && has_intersection_aux(
                inv_dir, r, *node.interior.left))
                return true;

            const bool inct_right = node.interior.right_bound.intersect(
                r.o, inv_dir, r.t_min, r.t_max);
            if(inct_right && has_intersection_aux(
                inv_dir, r, *node.interior.right))
                return true;

            return false;
        }

        for(size_t i = node.leaf.start; i < node.leaf.end; ++i)
        {
            if(prims_[i]->has_intersection(r))
                return true;
        }

        return false;
    }

    bool closest_intersection_aux(
        const FVec3 &inv_dir, Ray &r, const Node &node,
        EntityIntersection *inct) const noexcept
    {
        if(node.is_interior)
        {
            const bool try_inct_left = node.interior.left_bound.intersect(
                r.o, inv_dir, r.t_min, r.t_max);

            bool ret = try_inct_left && closest_intersection_aux(
                inv_dir, r, *node.interior.left, inct);

            const bool try_inct_right = node.interior.right_bound.intersect(
                r.o, inv_dir, r.t_min, r.t_max);

            ret |= try_inct_right && closest_intersection_aux(
                inv_dir, r, *node.interior.right, inct);

            return ret;
        }

        bool ret = false;

        for(size_t i = node.leaf.start; i < node.leaf.end; ++i)
        {
            if(prims_[i]->closest_intersection(r, inct))
            {
                r.t_max = inct->t;
                ret = true;
            }
        }

        return ret;
    }

public:

    explicit EntityBVHEmbree(int max_leaf_size)
        : max_leaf_size_(max_leaf_size), bvh_(nullptr), root_(nullptr)
    {
        
    }

    ~EntityBVHEmbree()
    {
        if(bvh_)
            rtcReleaseBVH(bvh_);
    }

    void build(const std::vector<RC<const Entity>> &entities) override
    {
        if(bvh_)
        {
            rtcReleaseBVH(bvh_);
            bvh_  = nullptr;
            root_ = nullptr;
        }

        prims_.clear();
        entities_ = entities;

        bvh_ = rtcNewBVH(embree_device());

        std::vector<RTCBuildPrimitive> build_prims(entities.size());
        for(size_t i = 0; i < entities_.size(); ++i)
        {
            auto &p = build_prims[i];
            auto &e = entities_[i];

            const auto bbox = e->world_bound();

            p.geomID  = static_cast<unsigned>(i);
            p.primID  = static_cast<unsigned>(i);
            p.lower_x = bbox.low.x;
            p.lower_y = bbox.low.y;
            p.lower_z = bbox.low.z;
            p.upper_x = bbox.high.x;
            p.upper_y = bbox.high.y;
            p.upper_z = bbox.high.z;
        }

        BuildingParams building_params = {
            prims_, entities_
        };

        RTCBuildArguments args = rtcDefaultBuildArguments();
        args.byteSize               = sizeof(args);
        args.buildFlags             = RTC_BUILD_FLAG_NONE;
        args.buildQuality           = RTC_BUILD_QUALITY_HIGH;
        args.maxBranchingFactor     = 2;
        args.maxDepth               = 128;
        args.sahBlockSize           = 1;
        args.minLeafSize            = 1;
        args.maxLeafSize            = max_leaf_size_;
        args.traversalCost          = 1;
        args.intersectionCost       = 1;
        args.bvh                    = bvh_;
        args.primitives             = build_prims.data();
        args.primitiveCount         = build_prims.size();
        args.primitiveArrayCapacity = build_prims.capacity();
        args.createNode             = create_interior;
        args.setNodeChildren        = set_interior_children;
        args.setNodeBounds          = set_interior_bounds;
        args.createLeaf             = create_leaf;
        args.userPtr                = &building_params;

        root_ = static_cast<Node*>(rtcBuildBVH(&args));
    }

    bool has_intersection(const Ray &r) const noexcept override
    {
        const FVec3 inv_dir = FVec3(1) / r.d;
        return has_intersection_aux(inv_dir, r, *root_);
    }

    bool closest_intersection(
        const Ray &r, EntityIntersection *inct) const noexcept override
    {
        const FVec3 inv_dir = FVec3(1) / r.d;
        Ray ray = r;
        return closest_intersection_aux(inv_dir, ray, *root_, inct);
    }
};

RC<Aggregate> create_entity_bvh(int max_leaf_size)
{
    return newRC<EntityBVHEmbree>(max_leaf_size);
}

RC<Aggregate> create_entity_bvh_embree(int max_leaf_size)
{
    return newRC<EntityBVHEmbree>(max_leaf_size);
}

AGZ_TRACER_END

#endif // #ifdef USE_EMBREE
