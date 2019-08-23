#pragma once

#include <atomic>

#include <agz/tracer_utility/math.h>
#include <agz/utility/alloc.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

namespace pgpt
{

inline bool in_canonical(const Vec2 &pnt) noexcept
{
    return 0 <= pnt.x && pnt.x <= 1 && 0 <= pnt.y && pnt.y <= 1;
}

struct QuadTreeArea
{
    Vec2 low, high;
    real value = 0;
};

// 用来近似[0, 1]^2上的函数的adaptive quad tree
// 函数值越大的地方划分越细
class QuadTree : public misc::uncopyable_t
{
    struct QuadTreeLeaf;
    struct QuadTreeInterior;

    using QuadTreeNode = misc::variant_t<QuadTreeLeaf, QuadTreeInterior>;

    struct QuadTreeLeaf
    {
        real sum       = 0;
        uint32_t count = 0;

        real value() const noexcept
        {
            return count ? sum / count : real(0);
        }
    };

    struct QuadTreeInterior
    {
        QuadTreeNode *children[4] = { nullptr, nullptr, nullptr, nullptr };
    };

    using Leaf     = QuadTreeLeaf;
    using Interior = QuadTreeInterior;
    using Node     = QuadTreeNode;
    using Pool     = alloc::obj_pool_t<Node>;

    Pool pool_;
    Node *root_;

    // 取得pnt对应的孩子节点下标
    // 以(0.5, 0.5)为原点，0，1，2，3分别为第一，二，三，四象限
    // 并将pnt变换到对应象限中的[0, 1]^2范围
    static int child_index(Vec2 &pnt)
    {
        if(pnt.x >= real(0.5))
        {
            pnt.x = 2 * (pnt.x - real(0.5));

            if(pnt.y >= real(0.5))
            {
                pnt.y = 2 * (pnt.y - real(0.5));
                return 0;
            }

            pnt.y *= 2;
            return 3;
        }

        pnt.x *= 2;

        if(pnt.y >= real(0.5))
        {
            pnt.y = 2 * (pnt.y - real(0.5));
            return 1;
        }

        pnt.y *= 2;
        return 2;
    }

    // 找出包含pnt的叶节点，以及它的面积
    Leaf *find_leaf(Vec2 pnt)
    {
        Node *node = root_;
        for(;;)
        {
            if(auto leaf = node->as_if<Leaf>())
                return leaf;
            node = node->as<Interior>().children[child_index(pnt)];
        }
    }

    // 已知node对应的面积为area，求area上node表示的函数的积分
    static real node_integral(const Node *node, real area)
    {
        return match_variant(*node,
            [=](const Leaf &leaf)
        {
            return area * leaf.value();
        },
            [=](const Interior &interior)
        {
            real ret = 0;
            real child_area = real(0.25) * area;
            for(int i = 0; i < 4; ++i)
                ret += node_integral(interior.children[i], child_area);
            return ret;
        });
    }

    // 若node是叶节点，且上面的积分超过阈值，则递归地将其划分成子树直到子树的每个叶节点上的积分都不超过阈值
    // 若node是内部节点且上面的积分不超过阈值，则将其收拢成一个叶节点
    static real refine_aux(Node *node, real area, real threshold, Pool &pool)
    {
        if(auto leaf = node->as_if<Leaf>())
            return refine_leaf(node, leaf, area, threshold, pool);
        
        auto &interior = node->as<Interior>();
        real ret = 0, child_area = real(0.25) * area;
        for(int i = 0; i < 4; ++i)
            ret += refine_aux(interior.children[i], child_area, threshold, pool);

        if(ret <= threshold)
        {
            uint32_t new_count = interior.children[0]->as<Leaf>().count
                               + interior.children[1]->as<Leaf>().count
                               + interior.children[2]->as<Leaf>().count
                               + interior.children[3]->as<Leaf>().count;
            real new_sum = ret * new_count / area;

            *node = Leaf{ new_sum, new_count };
        }

        return ret;
    }

    // 若leaf上的积分超过阈值，将其一分为四，并递归地处理四个孩子节点
    // 返回leaf上的积分总值
    static real refine_leaf(Node *node, Leaf *leaf, real area, real threshold, Pool &pool)
    {
        assert(leaf == &node->as<Leaf>());

        real integral = leaf->value() * area;
        if(integral <= threshold)
            return integral;

        auto interior = Interior();
        real child_area = real(0.25) * area;
        for(int i = 0; i < 4; ++i)
        {
            interior.children[i] = pool.create(Leaf{ leaf->sum, leaf->count });
            refine_leaf(interior.children[i], &interior.children[i]->as<Leaf>(), child_area, threshold, pool);
        }
        *node = interior;
        return integral;
    }

    // 以pool为分配器复制node的子树
    static Node *copy_from_node(const Node *node, Pool &pool)
    {
        return match_variant(*node,
            [&](const Leaf &leaf)
        {
            return pool.create(Leaf{ leaf.sum, leaf.count });
        },
            [&](const Interior &interior)
        {
            Interior new_interior;
            for(int i = 0; i < 4; ++i)
                new_interior.children[i] = copy_from_node(interior.children[i], pool);
            return pool.create(new_interior);
        });
    }

    // 以pool为分配器复制node的子树的结构，而叶子节点内的各记录都会被清零
    static Node *copy_structure_from_node(const Node *node, Pool &pool)
    {
        return match_variant(*node,
            [&](const Leaf &)
        {
            return pool.create(Leaf{ 0, 0 });
        },
            [&](const Interior &interior)
        {
            Interior new_interior;
            for(int i = 0; i < 4; ++i)
                new_interior.children[i] = copy_structure_from_node(interior.children[i], pool);
            return pool.create(new_interior);
        });
    }

    // 输出所有叶节点代表的区域及其上的函数值
    template<typename OutIterator>
    static void leaf_areas_aux(const Node *node, const Vec2 &low, const Vec2 &high, OutIterator &itor)
    {
        match_variant(*node,
            [&](const Leaf &leaf)
        {
            *itor++ = QuadTreeArea{ low, high, leaf.value() };
        },
            [&](const Interior &interior)
        {
            Vec2 mid = real(0.5) * (low + high);
            leaf_areas_aux(interior.children[0], mid, high, itor);
            leaf_areas_aux(interior.children[1], { low.x, mid.y }, { mid.x, high.y }, itor);
            leaf_areas_aux(interior.children[2], low, mid, itor);
            leaf_areas_aux(interior.children[3], { mid.x, low.y }, { high.x, mid.y }, itor);
        });
    }

public:

    QuadTree()
    {
        root_ = pool_.create(QuadTreeInterior());
        auto &root_interior = root_->as<QuadTreeInterior>();
        for(int i = 0; i < 4; ++i)
            root_interior.children[i] = pool_.create(QuadTreeLeaf());
    }

    // 记录一个新的采样点，该点将被用于估计它所在的叶节点区域的函数值
    void record(const Vec2 &pnt, real val)
    {
        assert(in_canonical(pnt));
        auto leaf = find_leaf(pnt);
        leaf->sum += val;
        leaf->count += 1;
    }

    // 整个[0, 1]^2上的积分值
    real total_integral() const
    {
        return node_integral(root_, 1);
    }

    // 对积分值超过阈值的叶节点，递归地划分之直到子树中地叶节点积分制都不超过阈值
    // 对积分值没超过阈值的内部节点，将其剪枝成为叶节点
    void refine(real threshold)
    {
        refine_aux(root_, 1, threshold, pool_);
    }

    // 原样复制一颗quad tree
    void copy_from(const QuadTree &tree)
    {
        pool_.free_all_memory();
        root_ = copy_from_node(tree.root_, pool_);
    }

    // 原样复制一颗quad tree的结构，叶节点中的数据都被清零
    void copy_structure_from(const QuadTree &tree)
    {
        pool_.free_all_memory();
        root_ = copy_structure_from_node(tree.root_, pool_);
    }

    // 以QuadTreeArea的形式取得所有的叶子区域
    template<typename OutIterator>
    void all_leaf_areas(OutIterator itor) const
    {
        leaf_areas_aux(root_, Vec2(0, 0), Vec2(1, 1), itor);
    }

    // 取得pnt所在的叶节点的面积以及其上的函数值
    std::pair<real, real> leaf_area_and_value(Vec2 pnt) const
    {
        const Node *node = root_;
        real area = 1;

        while(auto interior = node->as_if<Interior>())
        {
            node = interior->children[child_index(pnt)];
            area *= real(0.25);
        }

        return { area, node->as<Leaf>().value() };
    }
};

} // namespace pgpt

AGZ_TRACER_END
