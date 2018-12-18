#pragma once

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

    struct SampleLsResult
    {
        MediumPoint medPnt;
        Real medPdf; // medPdf为0表示endPdf有效
        Real endPdf;
    };

    virtual  SampleLsResult SampleLs(const Ray &r) const = 0;

    virtual MediumShadingPoint Shade(const MediumPoint &medPnt, Arena &arena) const = 0;
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
