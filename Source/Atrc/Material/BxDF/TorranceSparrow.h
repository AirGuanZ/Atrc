#pragma once

#include <Atrc/Material/BxDF/BxDF.h>
#include <Atrc/Material/Fresnel.h>

AGZ_NS_BEG(Atrc)

struct MDSampleResult
{
    Vec3 wi;
    Real pdf;
};

// 微表面分布函数的所有参数以及返回值均定义在宏观表面局部坐标系中
class MicrofacetDistribution
{
public:

    virtual ~MicrofacetDistribution() = default;

    virtual float Eval(const Vec3 &H) const = 0;

    virtual Option<MDSampleResult> SampleWi(const Vec3 &wo) const = 0;

    virtual Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo) const = 0;
};

class TorranceSparrow : public BxDF
{
    Spectrum rc_;
    const MicrofacetDistribution *md_;
    const Fresnel *fresnel_;

    float G(const Vec3 &wi, const Vec3 &wo, const Vec3 &H) const;

public:

    TorranceSparrow(const Spectrum &rc, const MicrofacetDistribution *md, const Fresnel *fresnel);

    Spectrum Eval(const LocalCoordSystem &localShdCoord, const Vec3 &wi, const Vec3 &wo) const override;

    Option<BxDFSampleWiResult> SampleWi(const LocalCoordSystem &localShdCoord, const Vec3 &wo) const override;

    Real SampleWiPDF(const LocalCoordSystem &localShdCoord, const Vec3 &wi, const Vec3 &wo) const override;
};

AGZ_NS_END(Atrc)
