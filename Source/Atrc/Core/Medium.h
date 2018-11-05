#pragma once

#include <Atrc/Core/Common.h>

AGZ_NS_BEG(Atrc)

struct PhaseFunctionSampleWiResult
{
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
	Real t;
	Vec3 pos;
	Vec3 wo;
	Medium *medium;
};

struct MediumShadingPoint
{
	const PhaseFunction *ph;
};

class Medium
{
public:

	virtual ~Medium() = default;

	virtual Spectrum Tr(const Ray &r) const = 0;

	virtual void Shade(const MediumPoint &medPnt, MediumShadingPoint *shdPnt, AGZ::ObjArena<> &arena) const = 0;
};

AGZ_NS_END(Atrc)
