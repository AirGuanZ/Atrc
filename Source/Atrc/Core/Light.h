#pragma once

#include <Atrc/Core/Common.h>

AGZ_NS_BEG(Atrc)

struct LightSampleToResult
{
    Vec3 pos;
    Vec3 wi;
    Spectrum radiance;
    Real pdf = 0.0;
};

class Light
{
public:

    virtual ~Light() = default;

    // 预处理场景以获得必要的信息，比如有向光获取场景大小以进行采样等
    // 缺省啥也不做
    virtual void PreprocessScene(const Scene &scene);

    // 在光源上采样一个点照向sp，原则上包含了AreaLe和NonareaLe两部分
    virtual LightSampleToResult SampleLi(const SurfacePoint &sp) const = 0;

    // 给定光源上的采样结果，求pdf
    // posOnLight指出pos是否在光源实体上，前者对应AreaLe，后者对应NonareaLe
    virtual Real SampleLiPDF(const Vec3 &pos, const SurfacePoint &dst, bool posOnLight = true) const = 0;

    // 位置采样是否使用delta分布
    virtual bool IsDeltaPosition() const = 0;

    // 是否是delta方向的光源，比如理想有向光
    virtual bool IsDeltaDirection() const = 0;

    virtual bool IsDelta() const;

    // 仅针对有实体的光源，设某r与实体交于sp，求这一点沿-r.dir的radiance
    virtual Spectrum AreaLe(const SurfacePoint &sp) const = 0;

    virtual bool IgnoreFirstMedium() const;

    // 没有实体，沿-r.dir方向凭空产生的那些le，比如skylight等
    virtual Spectrum NonareaLe(const Ray &r) const = 0;

    // 用来在多个光源间进行选择
    virtual Spectrum Power() const = 0;
};

class GeometricLight : public Light
{
protected:

    const Geometry *geometry_;

public:

    explicit GeometricLight(const Geometry *geometry)
        : geometry_(geometry)
    {
        AGZ_ASSERT(geometry);
    }

    const Geometry *GetGeometry() const
    {
        return geometry_;
    }
};

AGZ_NS_END(Atrc)
