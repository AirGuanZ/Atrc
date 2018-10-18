#pragma once

#include <Atrc/Core/Common.h>

AGZ_NS_BEG(Atrc)

struct LightSampleToResult
{
    Vec3 pos;
    Vec3 nor;
    Spectrum le;
    Real pdf = 0.0;
};

class Light
{
public:

    virtual ~Light() = default;

    // 在光源上采样一个点照向sp
    virtual LightSampleToResult SampleTo(const SurfacePoint &sp) const = 0;

    // 给定光源上的采样结果，求pdf
    virtual Real SampleToPDF(const SurfacePoint &sp, const Vec3 &pos) const = 0;

    // 位置采样是否使用delta分布
    virtual bool IsDeltaPosition() const = 0;

    // 是否是delta方向的光源，比如理想有向光
    virtual bool IsDeltaDirection() const = 0;

    // 仅针对有实体的光源，设某r与实体交于sp，求这一点沿-r.dir的radiance
    virtual Spectrum AreaLe(const SurfacePoint &sp, const Vec3 &wo) const = 0;

    // 没有实体，凭空产生的那些le，比如environment map
    virtual Spectrum Le(const Ray &r) const = 0;

    // 用来在多个光源间进行选择
    virtual Real Power() const = 0;
};

AGZ_NS_END(Atrc)
