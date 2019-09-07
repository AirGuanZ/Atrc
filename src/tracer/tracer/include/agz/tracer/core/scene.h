#pragma once

#include <agz/tracer/core/aggregate.h>
#include <agz/tracer/core/scattering.h>
#include <agz/tracer_utility/math.h>
#include <agz/tracer_utility/object.h>

AGZ_TRACER_BEGIN

class Camera;
class Entity;
class Light;
class EnvirLight;
class AreaLight;

struct EntityIntersection;
class ScatteringPoint;

/**
 * @brief 采样光源的结果
 */
struct SceneSampleLightResult
{
    const Light *light;
    real pdf;
};

/**
 * @brief scene::next_scattering_point的输出
 */
struct SampleScatteringResult
{
    ScatteringPoint pnt;                  // 散射点
    real pdf;                             // 采样到该点的pdf
    bool *p_has_inct           = nullptr; // 用于输出该r与场景实体是否有交点，可为nullptr
    EntityIntersection *p_inct = nullptr; // 用于输出该r与场景实体的首个交点，在p_has_inct不为空时必须不为空
};

/**
 * @brief 场景管理器接口
 *
 * 除非专门指明，否则Scene不拥有任何传递给它的Object的所有权
 */
class Scene : public obj::Object
{
public:

    using Object::Object;

    /** @brief 设置当前使用的摄像机 */
    virtual void set_camera(const Camera *camera) = 0;

    /** @brief 当前使用的摄像机，若未设置过则返回nullptr */
    virtual const Camera *camera() const noexcept = 0;

    /** @brief 在场景中添加一个新的实体 */
    virtual void add_light(Light *light) = 0;

    /** @brief 设置entity aggregate */
    virtual void set_aggregate(const Aggregate *aggregate) = 0;

    /** @brief 设置环境光源 */
    virtual void set_env_light(EnvirLight *env_light) = 0;

    /** @brief 场景中光源的数量 */
    virtual size_t light_count() const noexcept = 0;

    /** @brief 场景中的第idx个光源 */
    virtual const Light *light(size_t idx) const noexcept = 0;

    /** @brief 环境光 */
    virtual const EnvirLight *env() const noexcept = 0;

    /** @brief 场景中的所有光源 */
    virtual misc::span<const Light* const> lights() const noexcept = 0;

    /** @brief 随机选择场景中的一个光源 */
    virtual SceneSampleLightResult sample_light(const Sample1 &sam) const noexcept = 0;

    /**
     * @brief sample_light方法选中light的概率
     *
     * 若light不属于该场景，则返回0
     */
    virtual real light_pdf(const AreaLight *light) const noexcept = 0;

    /** @brief 给定射线是否与场景中的实体有交点 */
    virtual bool has_intersection(const Ray &r) const noexcept = 0;

    /**
     * @brief 两个点之间是否有障碍物
     */
    virtual bool visible(const Vec3 &A, const Vec3 &B) const noexcept
    {
        real dis = (A - B).length();
        Ray shadow_ray(A, (B - A).normalize(), EPS, dis - EPS);
        return !has_intersection(shadow_ray);
    }

    virtual bool visible_inf(const Vec3 &a, const Vec3 &dir) const noexcept
    {
        Ray shadow_ray(a, dir.normalize(), EPS);
        return !has_intersection(shadow_ray);
    }

    /**
     * @brief 给定射线与场景中实体的最近交点
     *
     * 只有在交点存在时inct中的数据才是有效的
     *
     * @return 交点是否存在
     */
    virtual bool closest_intersection(const Ray &r, EntityIntersection *inct) const noexcept = 0;

    /**
     * @brief 采样下一个散射事件
     */
    virtual bool next_scattering_point(const Ray &r, SampleScatteringResult *result, const Sample1 &sam, Arena &arena) const = 0;

    /**
     * @brief 确定o所处的medium
     */
    virtual const Medium *determine_medium(const Vec3 &o, const Vec3 &d = Vec3(0, 1, 0)) const noexcept = 0;

    /**
     * @brief 最小的包含所有实体的AABB盒
     */
    virtual AABB world_bound() const noexcept = 0;

    /** @brief 开始渲染前的回调，用于做各种必要的初始化 */
    virtual void start_rendering() = 0;
};

AGZT_INTERFACE(Scene)

AGZ_TRACER_END
