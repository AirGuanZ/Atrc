#pragma once

#include <Atrc/Core/Medium.h>

AGZ_NS_BEG(Atrc)

class HomogeneousMedium : public Medium
{
	// 吸收
	Spectrum sigmaA_;

	// 散射
	Spectrum sigmaS_;

	// 衰减（ = sigmaA_ + sigmaS_）
	Spectrum sigmaT_;

	// 自发光
	Spectrum le_;

	// 不对称系数
	Real g_;

public:

	HomogeneousMedium(const Spectrum &sigmaA, const Spectrum &sigmaS, const Spectrum &le, Real g);

	Spectrum Tr(const Vec3 &a, const Vec3 &b) const override;

	Either<MediumSampleLsResult, Real> SampleLs(const Ray &r) const override;

	void Shade(const MediumPoint &medPnt, MediumShadingPoint *shdPnt, AGZ::ObjArena<> &arena) const override;
};

AGZ_NS_END(Atrc)
