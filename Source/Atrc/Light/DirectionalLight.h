#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class DirectionalLight : public Light
{
    Vec3 dir_;
    Spectrum radiance_;
    
    Vec3 worldCentre_;
    Real worldRadius_;

public:

    DirectionalLight(const Vec3 &dir, const Spectrum &radiance);

    void PreprocessScene(const Scene &scene) override;

    // 在光源上采样一个点照向sp，原则上包含了AreaLe和NonareaLe两部分
    LightSampleToResult SampleLi(const SurfacePoint &sp) const override;

    // 给定光源上的采样结果，求pdf
    // posOnLight指出pos是否在光源实体上，前者对应AreaLe，后者对应NonareaLe
    Real SampleLiPDF(const Vec3 &pos, const Vec3 &dst, bool posOnLight) const override;

    // 位置采样是否使用delta分布
    bool IsDeltaPosition() const override;

    // 是否是delta方向的光源，比如理想有向光
    bool IsDeltaDirection() const override;

    bool IsDelta() const override;

    // 仅针对有实体的光源，设某r与实体交于sp，求这一点沿-r.dir的radiance
    Spectrum AreaLe(const SurfacePoint &sp) const override;

    // 没有实体，沿-r.dir方向凭空产生的那些le，比如skylight等
    Spectrum NonareaLe(const Ray &r) const override;

    // 用来在多个光源间进行选择
    Spectrum Power() const override;
};

AGZ_NS_END(Atrc)
