#include <Atrc/Lib/Core/CoordSystem.h>
#include <Atrc/Lib/Core/Ray.h>
#include <Atrc/Lib/Medium/HomogeneousMedium.h>

namespace Atrc
{

namespace
{
    // See http://www.astro.umd.edu/~jph/HG_note.pdf
    class HenyeyGreenstein : public PhaseFunction
    {
        Vec3 wo;
        Real g;

    public:

        explicit HenyeyGreenstein(const Vec3 &wo, Real g)
            : wo(wo.Normalize()), g(g) { }

        Real Eval(const Vec3& wi, const Vec3& wo) const override
        {
            Real cos = Cos(wi, wo);
            Real dem = 1 + g * g + 2 * g * cos;
            return 1 / (4 * PI) * (1 - g * g) / (dem * Sqrt(dem));
        }

        SampleWiResult SampleWi(const Vec2 &sample) const override
        {
            SampleWiResult ret;
            Real cosTheta;

            if(Abs(g) < EPS)
                cosTheta = 1 - 2 * sample.x;
            else
            {
                Real t = (1 - g * g) / (1 - g + 2 * g * sample.x);
                cosTheta = 1 / (2 * g) * (1 + g * g - t * t);
            }

            Real sinTheta = Sqrt(Max(Real(0), 1 - cosTheta * cosTheta));

            Real phi = 2 * PI * sample.y;

            auto dem = Real(1 + g * g + 2 * g * cosTheta);
            ret.coef = Real(1 / (4 * PI) * (1 - g * g) / (dem * Sqrt(dem)));
            ret.wi = CoordSystem::FromEz(-wo).Local2World(
                { sinTheta * Cos(phi), sinTheta * Sin(phi), cosTheta });

            return ret;
        }
    };
}

HomogeneousMedium::HomogeneousMedium(
    const Spectrum &sigmaA, const Spectrum &sigmaS, const Spectrum &le, Real g) noexcept
    : sigmaT_(sigmaA + sigmaS), sigmaS_(sigmaS), le_(le), g_(g)
{

}

Spectrum HomogeneousMedium::Tr(const Vec3 &a, const Vec3 &b) const
{
#ifdef AGZ_CC_GCC
    return (-sigmaT_ * Real((a - b).Length())).Map<decltype(&Exp<Real>)>(&Exp<Real>);
#else
    return (-sigmaT_ * Real((a - b).Length())).Map(Exp<Real>);
#endif
}

Spectrum HomogeneousMedium::TrToInf([[maybe_unused]] const Vec3 &a, [[maybe_unused]] const Vec3 &d) const
{
    return Spectrum(!!sigmaT_ ? Real(0) : Real(1));
}

Medium::SampleLsResult HomogeneousMedium::SampleLs(const Ray &r, const Vec3 &sample) const
{
    AGZ_ASSERT(ApproxEq(r.d.Length(), Real(1), Real(1e-3)));

    if(!sigmaS_ && !le_)
        return SampleLsResult(Real(1));

    Real tMax = r.t1 - r.t0;

    int channel = AGZ::Math::DistributionTransform::UniformInteger<Real>
        ::Transform<int>(sample.x, 0, SPECTRUM_CHANNEL_COUNT);
    Real st = -Log_e(sample.y) / sigmaT_[channel];
    bool sampleMedium = st < tMax;

#ifdef AGZ_CC_GCC
    auto Tr = (-sigmaT_ * Min(st, tMax)).Map<decltype(&Exp<Real>)>(&Exp<Real>);
#else
    auto Tr = (-sigmaT_ * Min(st, tMax)).Map(Exp<Real>);
#endif
    auto density = sampleMedium ? (sigmaT_ * Tr) : Tr;

    Real pdf = 0;
    for(int i = 0; i < SPECTRUM_CHANNEL_COUNT; ++i)
        pdf += density[i];
    pdf *= Real(1) / SPECTRUM_CHANNEL_COUNT;
    pdf = Max(pdf, EPS);

    if(!sampleMedium)
        return pdf;
    
    MediumLsSample ret;
    ret.pnt.t      = st + r.t0;
    ret.pnt.pos    = r.At(st + r.t0);
    ret.pnt.wo     = -r.d;
    ret.pnt.medium = this;
    ret.pdf        = pdf;

    return ret;
}

MediumShadingPoint HomogeneousMedium::GetShadingPoint(const MediumPoint &medPnt, Arena &arena) const
{
    MediumShadingPoint ret;
    ret.ph     = arena.Create<HenyeyGreenstein>(medPnt.wo, g_);
    ret.le     = le_;
    ret.sigmaS = sigmaS_;
    return ret;
}

} // namespace Atrc
