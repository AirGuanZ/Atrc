#pragma once

#include "./dtree.h"

AGZ_TRACER_BEGIN

namespace pgpt
{

class STree : public misc::uncopyable_t
{
    struct Leaf;
    struct Interior;

    using Node = misc::variant_t<Leaf, Interior>;

    struct Leaf
    {
        uint32_t record_count = 0;
        DTrainer trainer;
        std::shared_ptr<DSampler> sampler;
    };

    struct Interior
    {
        Node *children[2] = { nullptr, nullptr };
    };

    using Pool = alloc::obj_pool_t<Node>;

    Pool pool_;
    Node *root_;

    AABB world_bound_;

    // 0 -> 1, 1 -> 2, 2 -> 0
    static int next_axis(int axis) noexcept
    {
        return (axis + 1) % 3;
    }

    // 找到包含pos的叶节点
    Leaf *leaf_contains(const Vec3 &pos) const
    {
        AABB bbox = world_bound_;
        Node *node = root_;
        int axis = 0;

        for(;;)
        {
            if(auto leaf = node->as_if<Leaf>())
                return leaf;

            auto &interior = node->as<Interior>();
            real mid = real(0.5) * (bbox.low[axis] + bbox.high[axis]);
            if(pos[axis] < mid)
            {
                node = interior.children[0];
                bbox.high[axis] = mid;
            }
            else
            {
                node = interior.children[1];
                bbox.low[axis] = mid;
            }

            axis = next_axis(axis);
        }
    }

    // 将所有node下的叶节点的trainer做一次迭代，并更新其对应的sampler
    static void iterate_trainer_and_sampler(real rho, Node *node)
    {
        match_variant(*node,
            [=](Leaf &leaf)
        {
            leaf.sampler = leaf.trainer.iterate(rho);
        },
            [=](Interior &interior)
        {
            iterate_trainer_and_sampler(rho, interior.children[0]);
            iterate_trainer_and_sampler(rho, interior.children[1]);
        });
    }

    // 递归地划分记录数量超过了阈值的叶节点
    static void iterate_snode(Node *node, uint32_t threshold, Pool &pool)
    {
        if(auto leaf = node->as_if<Leaf>())
        {
            if(leaf->record_count > threshold)
            {
                uint32_t child_record_count = leaf->record_count / 2;
                DTrainer trainer = std::move(leaf->trainer);
                auto sampler = std::move(leaf->sampler);

                *node = Interior();
                auto &interior = node->as<Interior>();
                
                interior.children[0] = pool.create(Leaf());
                auto &child0 = interior.children[0]->as<Leaf>();
                child0.record_count = child_record_count;
                child0.sampler      = sampler;
                child0.trainer      = trainer.clone();

                interior.children[1] = pool.create(Leaf());
                auto &child1 = interior.children[1]->as<Leaf>();
                child1.record_count = child_record_count;
                child1.sampler      = std::move(sampler);
                child1.trainer      = std::move(trainer);

                iterate_snode(interior.children[0], threshold, pool);
                iterate_snode(interior.children[1], threshold, pool);
            }
        }
        else
        {
            auto &interior = node->as<Interior>();
            iterate_snode(interior.children[0], threshold, pool);
            iterate_snode(interior.children[1], threshold, pool);
        }
    }

    // 将所有叶节点的记录数量清零
    static void clear_record_count(Node *node)
    {
        match_variant(*node,
            [](Leaf &leaf)
        {
            leaf.record_count = 0;
        },
            [](Interior &interior)
        {
            clear_record_count(interior.children[0]);
            clear_record_count(interior.children[1]);
        });
    }

    // 将所有叶结点的trainer转换为sampler
    static void iterate_end_for_leaves(Node *node)
    {
        match_variant(*node,
            [](Leaf &leaf)
        {
            leaf.sampler = std::move(leaf.trainer).to_sampler();
        },
            [](Interior &interior)
        {
            iterate_end_for_leaves(interior.children[0]);
            iterate_end_for_leaves(interior.children[1]);
        });
    }

public:

    // 给定整个场景的BBox，后续的记录点都必须在这个box内
    explicit STree(const AABB &world_bound)
        : world_bound_(world_bound)
    {
        root_ = pool_.create(Leaf());
    }

    // 添加一个记录点
    void record(const Vec3 &pos, const Vec3 &dir, real val)
    {
        assert(world_bound_.contains(pos));
        auto leaf = leaf_contains(pos);
        leaf->trainer.record(dir, val);
        leaf->record_count += 1;
    }

    // 取得某个位置对应的DSampler
    const DSampler *direction_sampler(const Vec3 &pos) const
    {
        assert(world_bound_.contains(pos));
        auto leaf = leaf_contains(pos);
        assert(leaf->sampler);
        return leaf->sampler.get();
    }

    // 将上一轮的所有dtree转换为sampler，refine这些dtree，然后refine自身的结构，准备好下一轮训练
    void iterate(real dtree_rho, uint32_t record_count_threshold)
    {
        iterate_trainer_and_sampler(dtree_rho, root_);
        iterate_snode(root_, record_count_threshold, pool_);
        clear_record_count(root_);
    }

    // 将上一轮的所有dtree转换为sampler，转换后不能再调用direction_sampler以外的成员函数
    void iterate_end()
    {
        iterate_end_for_leaves(root_);
    }
};

} // namespace pgpt

AGZ_TRACER_END
