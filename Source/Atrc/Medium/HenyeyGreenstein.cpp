#include <Atrc/Medium/HenyeyGreenstein.h>

AGZ_NS_BEG(Atrc)

HenyeyGreenstein::HenyeyGreenstein(const Vec3 &wo, Real g)
	: wo(wo), g(g)
{

}

PhaseFunctionSampleWiResult HenyeyGreenstein::SampleWi() const
{
	PhaseFunctionSampleWiResult ret;
	Real u = Rand(), v = Rand(), cosTheta;

	if(Abs(g) < EPS)
		cosTheta = 1 - 2 * u;
	else
	{
		Real t = (1 - g * g) / (1 - g + 2 * g * u);
		cosTheta = 1 / (2 * g) * (1 + g * g - t * t);
	}

	Real sinTheta = Sqrt(Max(Real(0), 1 - cosTheta * cosTheta));

	Real phi = 2 * PI * v;

	ret.wi = LocalCoordSystem::FromEz(-wo).Local2World(
		{ sinTheta * Cos(phi), sinTheta * Sin(phi), cosTheta });
	return ret;
}

AGZ_NS_END(Atrc)
