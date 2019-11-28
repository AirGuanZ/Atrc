#pragma once

#include <agz/utility/misc.h>

#include <agz/tracer/core/aggregate.h>
#include <agz/common/math.h>

AGZ_TRACER_BEGIN

class Camera;
class Entity;
class Light;
class AreaLight;
class NonareaLight;

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
 * @brief 场景管理器接口
 *
 * 除非专门指明，否则Scene不拥有任何传递给它的Object的所有权
 */
class Scene
{
public:

    virtual ~Scene() = default;

    /** @brief 设置当前使用的摄像机 */
    virtual void set_camera(std::shared_ptr<const Camera> camera) = 0;

    /** @brief 当前使用的摄像机，若未设置过则返回nullptr */
    virtual const Camera *camera() const noexcept = 0;

    /** @brief 场景中的所有光源 */
    virtual misc::span<const Light* const> lights() const noexcept = 0;

    /** @brief 场景中的所有非实体光源 */
    virtual misc::span<const NonareaLight* const> nonarea_lights() const noexcept = 0;

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

    /**
     * @brief 从a向dir方向看去，视线是否会被阻隔
     */
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
     * @brief 确定o所处的medium
     */
    virtual const Medium *determine_medium(const Vec3 &o, const Vec3 &d = Vec3(0, 1, 0)) const noexcept = 0;

    /**
     * @brief 最小的包含所有实体的AABB盒
     */
    virtual AABB world_bound() const noexcept = 0;
};

AGZ_TRACER_END
