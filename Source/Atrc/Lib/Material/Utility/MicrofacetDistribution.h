#pragma once

#include <Atrc/Lib/Core/Common.h>

namespace Atrc
{
    
// 所有传递给MicrofacetDistribution各成员函数的方向向量参数都必须是归一化的
class MicrofacetDistribution
{
public:

    virtual ~MicrofacetDistribution() = default;

    virtual Real D(const Vec3 &H) const noexcept = 0;

    virtual Real G(const Vec3 &H, const Vec3 &wi, const Vec3 &wo) const noexcept = 0;

    struct SampleWiResult
    {
        Vec3 wi;
        Real pdf;
    };

    virtual Option<SampleWiResult> SampleWi(const CoordSystem &geoInShd, const Vec3 &wo, const Vec2 &sample) const noexcept = 0;

    virtual Real SampleWiPDF(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo) const noexcept = 0;
};

} // namespace Atrc
