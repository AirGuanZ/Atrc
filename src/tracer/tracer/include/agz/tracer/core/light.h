#pragma once

#include <agz/tracer/core/intersection.h>
#include <agz/tracer_utility/object.h>

AGZ_TRACER_BEGIN

class Scene;
class AreaLight;
class EnvirLight;

class Light
{
public:

    virtual ~Light() = default;

    virtual bool is_area() const noexcept = 0;

    virtual const AreaLight *as_area() const noexcept { return nullptr; }

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
 * @brief 从光源对参考点进行采样的结果
 */
struct AreaLightSampleResult
{
    SurfacePoint spt;
    Spectrum radiance; // 从position照射向参考点的辐射亮度
    real pdf      = 0; // w.r.t. solid angle at ref
    bool is_delta = false;
};

inline const AreaLightSampleResult AREA_LIGHT_SAMPLE_RESULT_NULL = { { }, Spectrum(), 0, false };

///**
// * @brief 从光源发射的粒子信息
// */
//struct LightEmitResult
//{
//    SurfacePoint spt;
//    Vec3 dir;
//    Spectrum radiance;
//    real pdf_pos = 1; // w.r.t. light source area
//    real pdf_dir = 1; // w.r.t. solid angle
//
//    const Medium *medium = nullptr;
//};

/**
 * @brief 光源接口
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

    ///**
    // * @brief 采样一个从光源发射的粒子
    // */
    //virtual LightEmitResult emit(const Sample5 &sam) const noexcept = 0;
    //
    ///**
    // * @brief 采样到从给定位置发射向给定方向的粒子的概率密度函数值
    // */
    //virtual void emit_pdf(const SurfacePoint &spt, const Vec3 &light_to_out, real *pdf_pos, real *pdf_dir) const noexcept = 0;
};

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

class EnvirLight : public obj::Object, public Light
{
protected:

    real world_radius_ = 1;
    Vec3 world_centre_;

public:

    bool is_area() const noexcept override { return false; }

    const EnvirLight *as_envir() const noexcept override { return this; }

    void preprocess(const Scene &scene) override;

    virtual EnvirLightSampleResult sample(const Sample3 &sam) const noexcept = 0;

    virtual real pdf(const Vec3 &ref_to_light) const noexcept = 0;

    virtual Spectrum radiance(const Vec3 &ref_to_light) const noexcept = 0;
};

AGZT_INTERFACE(EnvirLight)

AGZ_TRACER_END
