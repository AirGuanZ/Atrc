#pragma once

#include <agz/tracer/core/intersection.h>
#include <agz/tracer/core/object.h>

AGZ_TRACER_BEGIN

class Scene;
class AreaLight;
class EnvirLight;

/**
 * @brief 光源接口
 * 
 * 全体光源被分为两类：实体光源和环境光源。
 * 实体光源在场景中有对应的Entity，环境光源则没有
 */
class Light
{
public:

    virtual ~Light() = default;

    /**
     * @brief 是否是有实体的光源
     */
    virtual bool is_area() const noexcept = 0;

    /**
     * @brief 返回其实体光源接口
     */
    virtual const AreaLight *as_area() const noexcept { return nullptr; }

    /**
     * @brief 返回其环境光源接口
     */
    virtual const EnvirLight *as_envir() const noexcept { return nullptr; }

    /**
     * @brief 发射的光通量
     */
    virtual Spectrum power() const noexcept = 0;

    /**
     * @brief 基于场景进行预处理，在渲染开始之前、场景准备完毕之后调用一次
     */
    virtual void preprocess(const Scene &scene) = 0;
};

/**
 * @brief 从实体光源对参考点进行采样的结果
 */
struct AreaLightSampleResult
{
    SurfacePoint spt;
    Spectrum radiance; // 从position照射向参考点的辐射亮度
    real pdf      = 0; // w.r.t. solid angle at ref
    bool is_delta = false;
};

/**
 * @brief 采样实体光源失败时的返回值
 */
inline const AreaLightSampleResult AREA_LIGHT_SAMPLE_RESULT_NULL = { { }, Spectrum(), 0, false };

/**
 * @brief 实体光源接口
 */
class AreaLight : public Light
{
public:

    bool is_area() const noexcept override { return true; }

    const AreaLight *as_area() const noexcept override { return this; }
    
    /**
     * @brief 采样一条照射到ref的射线
     * @param ref 参考点
     * @param sam 采样数据
     */
    virtual AreaLightSampleResult sample(const Vec3 &ref, const Sample5 &sam) const noexcept = 0;

    /**
     * @brief 光源表面某点朝指定方向的辐射亮度
     * 
     * @param spt 光源表面的点
     * @param light_to_out spt向外照射的方向
     * @return 沿该射线反方向的radiance
     */
    virtual Spectrum radiance(const SurfacePoint &spt, const Vec3 &light_to_out) const noexcept = 0;

    /**
     * @brief 采样到某条照射ref的射线的概率密度（w.r.t. solid angle）
     *
     * @param ref 参考点
     * @param spt 光源上的点
     */
    virtual real pdf(const Vec3 &ref, const SurfacePoint &spt) const noexcept = 0;
};

/**
 * @brief 从环境光源对参考点进行采样的结果
 */
struct EnvirLightSampleResult
{
    Vec3 ref_to_light;
    Spectrum radiance;
    real pdf = 0;
    bool is_delta = false;

    bool valid() const noexcept
    {
        return !!ref_to_light;
    }
};

/**
 * @brief 环境光源接口
 */
class EnvirLight : public obj::Object, public Light
{
protected:

    real world_radius_ = 1;
    Vec3 world_centre_;

public:

    bool is_area() const noexcept override { return false; }

    const EnvirLight *as_envir() const noexcept override { return this; }

    void preprocess(const Scene &scene) override;

    /**
     * @brief 采样一条照射到ref的射线
     * @param ref 参考点
     * @param sam 采样数据
     */
    virtual EnvirLightSampleResult sample(const Vec3 &ref, const Sample3 &sam) const noexcept = 0;

    /**
     * @brief 采样到某条照射ref的射线的概率密度（w.r.t. solid angle）
     *
     * @param ref 参考点
     * @param ref_to_light 射线方向
     */
    virtual real pdf(const Vec3 &ref, const Vec3 &ref_to_light) const noexcept = 0;

    /**
     * @brief 从ref_to_light方向照射到ref点的radiance
     */
    virtual Spectrum radiance(const Vec3 &ref, const Vec3 &ref_to_light) const noexcept = 0;
};

AGZT_INTERFACE(EnvirLight)

AGZ_TRACER_END
