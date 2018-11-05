#include <Atrc/Medium/HenyeyGreenstein.h>
#include <Atrc/Medium/HomogeneousMedium.h>

AGZ_NS_BEG(Atrc)

HomogeneousMedium::HomogeneousMedium(const Spectrum &sigmaA, const Spectrum &sigmaS, const Spectrum &le, Real g)
	: sigmaA_(sigmaA), sigmaS_(sigmaS), sigmaT_(sigmaA + sigmaS), le_(le), g_(g)
{

}

Spectrum HomogeneousMedium::Tr(const Vec3 &a, const Vec3 &b) const
{
	return (-sigmaT_ * float((a - b).Length())).Map(Exp<float>);
}

Either<MediumSampleLsResult, Real> HomogeneousMedium::SampleLs(const Ray &r) const
{
	AGZ_ASSERT(r.IsNormalized() && r.minT <= r.maxT);

	Real tMax = r.maxT - r.minT;

	int sampleChannel = AGZ::Math::Random::Uniform(0, SPECTRUM_CHANNEL_COUNT - 1);
	Real st = -Log_e(1 - Rand()) / sigmaT_[sampleChannel];
	bool sampleMedium = st <= tMax;

	auto Tr = (-sigmaT_ * Min(st, RealT::Max().Value())).Map(Exp<Real>);
	auto density = sampleMedium ? (sigmaT_.Map(AGZ::TypeOpr::StaticCaster<Real, float>) * Tr) : Tr;

	Real pdf = 0.0;
	for(int i = 0; i < SPECTRUM_CHANNEL_COUNT; ++i)
		pdf += density[i];
	pdf *= Real(1) / SPECTRUM_CHANNEL_COUNT;

	if(!sampleMedium)
		return pdf;

	MediumSampleLsResult ret;
	ret.medPnt.t      = st + r.minT;
	ret.medPnt.pos    = r.At(st + r.minT);
	ret.medPnt.wo     = -r.dir;
	ret.medPnt.medium = this;
	ret.pdf = pdf;

	return ret;
}

void HomogeneousMedium::Shade(const MediumPoint &medPnt, MediumShadingPoint *shdPnt, AGZ::ObjArena<> &arena) const
{
	AGZ_ASSERT(shdPnt && IsNormalized(medPnt.wo));
	shdPnt->ph = arena.Create<HenyeyGreenstein>(medPnt.wo, g_);
	shdPnt->le = le_;
	shdPnt->sigmaS = sigmaS_;
}

AGZ_NS_END(Atrc)
