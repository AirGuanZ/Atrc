#pragma once

#include <Atrc/Core/Common.h>

AGZ_NS_BEG(Atrc)

struct PhaseFunctionSampleWiResult
{
    float coef = float(0);
    Vec3 wi;
};

class PhaseFunction
{
public:

    virtual ~PhaseFunction() = default;

    virtual PhaseFunctionSampleWiResult SampleWi() const = 0;
};

struct MediumPoint
{
    Real t = Real(0);
    Vec3 pos;
    Vec3 wo;
    const Medium *medium = nullptr;
};

struct MediumShadingPoint
{
    const PhaseFunction *ph = nullptr;
    Spectrum le;
    Spectrum sigmaS;
};

struct MediumSampleLsResult
{
    MediumPoint medPnt;
    Real pdf = Real(0);
};

class Medium
{
public:

    virtual ~Medium() = default;

    virtual Spectrum Tr(const Vec3 &a, const Vec3 &b) const = 0;

    // 在r上成功采样时为L，采样失败时仅返回概率密度
    // r.dir为-wo
    virtual Either<MediumSampleLsResult, Real> SampleLs(const Ray &r) const = 0;

    virtual void Shade(const MediumPoint &medPnt, MediumShadingPoint *shdPnt, AGZ::ObjArena<> &arena) const = 0;
};

struct MediumInterface
{
    const Medium *in  = nullptr;
    const Medium *out = nullptr;

    const Medium *GetMedium(const Vec3 &norOut, const Vec3 &w) const
    {
        return Dot(norOut, w) > 0 ? out : in;
    }
};

AGZ_NS_END(Atrc)
