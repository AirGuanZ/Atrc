#pragma once

#include <Utils/Misc.h>

#include <Atrc/Lib/Core/Common.h>

namespace Atrc
{

struct MediumPoint
{
    Real t;
    Vec3 pos;
    Vec3 wo;
    const Medium *medium;
};

struct MediumShadingPoint
{
    const PhaseFunction *ph;
    Spectrum le;
    Spectrum sigmaS;
};
    
class Medium
{
public:

    virtual ~Medium() = default;

    virtual Spectrum Tr(const Vec3 &a, const Vec3 &b) const = 0;

    struct MediumLsSample { MediumPoint pnt; Real pdf; };

    // MediumLsSample为介质中的采样点，Real为采样到终点（非介质）时的概率密度函数值
    using SampleLsResult = Variant<MediumLsSample, Real>;

    // assert r is normalized
    virtual  SampleLsResult SampleLs(const Ray &r, const Vec3 &sample) const = 0;

    virtual MediumShadingPoint GetShadingPoint(const MediumPoint &medPnt, Arena &arena) const = 0;
};

class PhaseFunction
{
public:

    virtual ~PhaseFunction() = default;

    struct SampleWiResult
    {
        Real coef;
        Vec3 wi;
    };

    virtual SampleWiResult SampleWi() const = 0;
};

struct MediumInterface
{
    const Medium *in  = nullptr;
    const Medium *out = nullptr;

    const Medium *GetMedium(const Vec3 &normalOut, const Vec3 &wr) const;
};

// ================================= Implementation

inline const Medium *MediumInterface::GetMedium(const Vec3 &normalOut, const Vec3 &wr) const
{
    return Dot(normalOut, wr) > 0 ? out : in;
}

} // namespace Atrc
