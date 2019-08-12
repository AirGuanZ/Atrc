#pragma once

#include <iterator>
#include <vector>

#include "./quad_tree.h"

AGZ_TRACER_BEGIN

namespace pgpt
{

/**
 * @brief 将S^2上的点映射到[0, 1]^2上，保持任何地方的面积比都是4 * PI
 */
inline Vec2 direction_to_canonical(const Vec3 &dir) noexcept
{
    real phi = local_angle::phi(dir);
    real cos_theta = local_angle::cos_theta(dir.normalize());

    real u = math::clamp<real>(phi * (invPI_r / 2), 0, 1);
    real v = math::clamp<real>((cos_theta + 1) * real(0.5), 0, 1);

    return { u, v };
}

/**
 * @brief direction_to_canonical^-1
 */
inline Vec3 canonical_to_direction(const Vec2 &pnt) noexcept
{
    real phi = 2 * PI_r * pnt.x;
    real cos_theta = math::clamp<real>(pnt.y * 2 - 1, -1, 1);
    real sin_theta = local_angle::cos_2_sin(cos_theta);

    real x = std::cos(phi) * sin_theta;
    real y = std::sin(phi) * sin_theta;
    real z = cos_theta;

    return { x, y, z };
}

/**
 * @brief 用于采样单位球面上的方向
 */
class DSampler : public misc::uncopyable_t
{
    struct DSamplerArea
    {
        Vec2 low, delta;
        real integral_ratio = 1;
    };

    std::vector<DSamplerArea> areas_;
    math::distribution::alias_sampler_t<real> alias_sampler_;

    std::unique_ptr<QuadTree> qtree_;
    real total_integral_;

public:

    // 利用tree构造一个方向采样器
    // tree_total_integral用于提供tree上的总积分值；若为负，则自动调用tree->total_integral()计算
    explicit DSampler(std::unique_ptr<QuadTree> &&tree, real tree_total_integral = -1)
    {
        if(tree_total_integral < 0)
            tree_total_integral = tree->total_integral();
        total_integral_ = tree_total_integral;

        if(total_integral_ < EPS)
        {
            areas_.push_back({ Vec2(0, 0), Vec2(1, 1), 1 });
            real prob = 1;
            alias_sampler_.initialize(&prob, 1);
            qtree_ = std::move(tree);
            total_integral_ = 0;
            return;
        }

        std::vector<real> integrals_;
        std::vector<QuadTreeArea> tree_areas;
        tree->all_leaf_areas(std::back_inserter(tree_areas));

        integrals_.reserve(tree_areas.size());
        areas_.reserve(tree_areas.size());
        for(auto &area : tree_areas)
        {
            real integral = (area.high - area.low).product() * area.value;
            if(!integral)
                continue;
            real integral_ratio = integral / tree_total_integral;
            integrals_.push_back(integral);
            areas_.push_back({ area.low, area.high - area.low, integral_ratio });
        }

        alias_sampler_.initialize(integrals_.data(), static_cast<int>(integrals_.size()));
        qtree_ = std::move(tree);
    }

    DSampler(DSampler &&move_from) noexcept = default;

    DSampler &operator=(DSampler &&move_from) noexcept = default;

    // 采样一个方向，返回方向向量和对应pdf
    std::pair<Vec3, real> sample(const Sample4 &sam) const
    {
        int area_idx = alias_sampler_.sample(sam.u, sam.v);
        auto &area = areas_[area_idx];
        Vec2 pnt = area.low + area.delta * Vec2(sam.w, sam.r);
        Vec3 dir = canonical_to_direction(pnt);
        real pdf = area.integral_ratio / (4 * PI_r * area.delta.product());

        assert(std::abs(pdf) - this->pdf(dir) <= EPS);

        return { dir, pdf };
    }

    // 采样到dir的pdf
    real pdf(const Vec3 &dir) const
    {
        if(total_integral_ < EPS)
            return 1 / (4 * PI_r);

        Vec2 pnt = direction_to_canonical(dir);
        auto [area, value] = qtree_->leaf_area_and_value(pnt);
        return value / (4 * PI_r * total_integral_);
    }
};

/**
 * @brief S^2 \to \mathbb R上的分段常函数的训练器，可训练得到DSampler
 */
class DTrainer : public misc::uncopyable_t
{
    std::unique_ptr<QuadTree> qtree_;

    explicit DTrainer(std::unique_ptr<QuadTree> &&qtree) noexcept
        : qtree_(std::move(qtree))
    {
        
    }

public:

    DTrainer()
    {
        qtree_ = std::make_unique<QuadTree>();
    }

    DTrainer(DTrainer&&)            noexcept = default;
    DTrainer &operator=(DTrainer&&) noexcept = default;

    // 进行一次训练迭代
    // 将之前的训练结果做成DSampler返回
    // 优化内部的quad tree结构并清空之前的训练数据
    std::shared_ptr<DSampler> iterate(real rho)
    {
        real total_integral = qtree_->total_integral();
        real threshold = rho * total_integral;
        qtree_->refine(threshold);

        auto new_qtree = std::make_unique<QuadTree>();
        new_qtree->copy_structure_from(*qtree_);

        auto ret = std::make_shared<DSampler>(std::move(qtree_), total_integral);
        qtree_ = std::move(new_qtree);

        return ret;
    }

    // 将之前的训练结果做成DSampler，调用该训练器立刻失效
    std::shared_ptr<DSampler> to_sampler() &&
    {
        real total_integral = qtree_->total_integral();
        return std::make_shared<DSampler>(std::move(qtree_), total_integral);
    }

    // 复制一个完全的训练器出来
    DTrainer clone() const
    {
        auto new_qtree = std::make_unique<QuadTree>();
        new_qtree->copy_from(*qtree_);
        return DTrainer(std::move(new_qtree));
    }

    // 添加一个训练数据
    void record(const Vec3 &dir, real val)
    {
        Vec2 pnt = direction_to_canonical(dir);
        qtree_->record(pnt, val);
    }
};

} // namespace pgpt

AGZ_TRACER_END
