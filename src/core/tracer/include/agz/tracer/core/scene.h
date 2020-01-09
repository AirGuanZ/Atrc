#pragma once

#include <agz/utility/misc.h>

#include <agz/tracer/core/aggregate.h>

AGZ_TRACER_BEGIN

class Camera;
class Entity;
class Light;
class AreaLight;
class EnvirLight;

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
    virtual const Camera *get_camera() const noexcept = 0;

    /** @brief 场景中的所有光源 */
    virtual misc::span<const Light* const> lights() const noexcept = 0;

    /** @brief 场景中的所有非实体光源 */
    virtual misc::span<const EnvirLight * const> envir_lights() const noexcept = 0;

    /**
     * @brief 随机选择场景中的一个光源
     *
     * 除非无光源，否则绝不会返回空值
     */
    virtual SceneSampleLightResult sample_light(const Sample1 &sam) const noexcept = 0;

    /**
     * @brief sample_light方法选中light的概率
     *
     * assert(light in scene)
     */
    virtual real light_pdf(const AreaLight *light) const noexcept = 0;

    /** @brief 给定射线是否与场景中的实体有交点 */
    virtual bool has_intersection(const Ray &r) const noexcept = 0;

    /**
     * @brief 两个点之间是否有障碍物
     */
    virtual bool visible(const Vec3 &A, const Vec3 &B) const noexcept = 0;

    /**
     * @brief 给定射线与场景中实体的最近交点
     *
     * 只有在交点存在时inct中的数据才是有效的
     *
     * @return 交点是否存在
     */
    virtual bool closest_intersection(const Ray &r, EntityIntersection *inct) const noexcept = 0;

    /**
     * @brief 预处理world bound、light source等
     */
    virtual void start_rendering() = 0;
};

AGZ_TRACER_END
