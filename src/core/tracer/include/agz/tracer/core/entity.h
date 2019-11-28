#pragma once

#include <string>

#include <agz/tracer/core/intersection.h>

AGZ_TRACER_BEGIN

class AreaLight;

/**
 * @brief 实体接口，表示场景中的可渲染物体
 * 
 * 所有的实体都被定义在世界坐标系中，原则上不允许实体间出现任何直接或间接的嵌套结构
 */
class Entity
{
    bool no_denoise_ = false;

public:

    virtual ~Entity() = default;

    void set_no_denoise_flag(bool no_denoise) noexcept { no_denoise_ = no_denoise; }

    bool get_no_denoise_flag() const noexcept { return no_denoise_; }

    /** @brief 判断给定射线是否与该实体有交点 */
    virtual bool has_intersection(const Ray &r) const noexcept = 0;

    /**
     * @brief 找到给定射线与该实体的最近交点
     * 
     * @param r 射线
     * @param inct 交点信息，其每个成员有效当且仅当返回值为true；若返回false，则inct必然未被修改
     * 
     * @return 存在交点时返回true，否则返回false
     */
    virtual bool closest_intersection(const Ray &r, EntityIntersection *inct) const noexcept = 0;

    /**
     * @brief 返回该实体在世界坐标系中的轴对齐包围盒
     */
    virtual AABB world_bound() const noexcept = 0;

    /**
     * @brief 返回自身作为area_light的接口。若并非光源，则返回nullptr。
     */
    virtual const AreaLight *as_light() const noexcept = 0;

    /**
     * @brief 返回自身作为area_light的接口。若并非光源，则返回nullptr。
     */
    virtual AreaLight *as_light() noexcept = 0;

    /**
     * @brief 是否是shadow catcher
     * 
     * shadow catcher效果需要renderer方面的支持
     */
    virtual bool is_shadow_catcher() const noexcept { return false; }
};

AGZ_TRACER_END
