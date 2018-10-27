#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

struct MDSampleResult
{
    Vec3 H;
    Real pdf;
};

// 微表面分布函数的所有参数以及返回值均定义在宏观表面局部坐标系中
class MicrofacetDistribution
{
public:

    virtual ~MicrofacetDistribution() = default;

    virtual float Eval(const Vec3 &H) const = 0;
};

AGZ_NS_END(Atrc)
