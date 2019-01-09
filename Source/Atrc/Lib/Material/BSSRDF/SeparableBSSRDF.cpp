#include <Atrc/Lib/Core/Entity.h>
#include <Atrc/Lib/Core/Material.h>
#include <Atrc/Lib/Core/Ray.h>
#include <Atrc/Lib/Material/BSSRDF/SeparableBSSRDF.h>
#include <Atrc/Lib/Material/Utility/Fresnel.h>

namespace Atrc
{

namespace
{
    [[maybe_unused]] Real FresnelMoment(Real eta)
    {
        Real eta2 = eta * eta, eta3 = eta2 * eta, eta4 = eta2 * eta2, eta5 = eta2 * eta3;
        if(eta < 1)
        {
            return Real(0.45966)
                 - Real(1.73965)  * eta
                 + Real(3.37668)  * eta2
                 - Real(3.904945) * eta3
                 + Real(2.49277)  * eta4
                 - Real(0.68441)  * eta5;
        }
        return Real(-4.61686)
             + Real(11.1136) * eta
             - Real(10.4646) * eta2
             + Real(5.11455) * eta3
             - Real(1.27198) * eta4
             + Real(0.12746) * eta5;
    }

    class SeparableBSSRDF_BSDF : public BSDF
    {
        Intersection pi_;
        const SeparableBSSRDF *bssrdf_;

    public:

        SeparableBSSRDF_BSDF(const Intersection &pi, const SeparableBSSRDF *bssrdf) noexcept
            : pi_(pi), bssrdf_(bssrdf)
        {
            AGZ_ASSERT(bssrdf);
        }

        Spectrum GetAlbedo([[maybe_unused]] BSDFType type) const noexcept override
        {
            return Spectrum();
        }

        Spectrum Eval(
            [[maybe_unused]] const CoordSystem &shd, [[maybe_unused]] const CoordSystem &geo,
            const Vec3 &wi, [[maybe_unused]] const Vec3 &wo,
            BSDFType type, bool star) const noexcept override
        {
            if(!((type & BSDF_TRANSMISSION) && (type & (BSDF_DIFFUSE | BSDF_GLOSSY))))
                return Spectrum();
            auto tpi = pi_;
            tpi.wr = wi;
            auto ret = bssrdf_->Eval(tpi, star);
            return ret;
        }

        Option<SampleWiResult> SampleWi(
            const CoordSystem &shd, const CoordSystem &geo,
            const Vec3 &wo, BSDFType type, bool star, const Vec2 &sample) const noexcept override
        {
            if(!((type & BSDF_TRANSMISSION) && (type & (BSDF_DIFFUSE | BSDF_GLOSSY))))
                return None;

            auto [sam, pdf] = AGZ::Math::DistributionTransform
                ::ZWeightedOnUnitHemisphere<Real>::Transform(sample);
            if(!pdf)
                return None;
            sam = shd.Local2World(sam);

            SampleWiResult ret;
            ret.coef    = Eval(shd, geo, sam, wo, type, star);
            ret.pdf     = pdf;
            ret.type    = BSDFType(BSDF_TRANSMISSION | BSDF_GLOSSY);
            ret.wi      = sam;
            ret.isDelta = false;

            if(!ret.coef || !ret.pdf)
                return None;

            return ret;
        }

        Real SampleWiPDF(
            const CoordSystem &shd, [[maybe_unused]] const CoordSystem &geo,
            const Vec3 &wi, const Vec3 &wo, BSDFType type, [[maybe_unused]] bool star) const noexcept override
        {
            if(!((type & BSDF_TRANSMISSION) && (type & (BSDF_DIFFUSE | BSDF_GLOSSY))))
                return 0;
            auto lwi = shd.World2Local(wi), lwo = shd.World2Local(wo);
            if(lwi.z <= 0 || lwo.z <= 0 || !geo.InPositiveHemisphere(wi))
                return 0;
            return AGZ::Math::DistributionTransform::ZWeightedOnUnitHemisphere<Real>::PDF(lwi.Normalize());
        }
    };

    class SeparableBSSRDF_BSDFMaterial : public Material
    {
        SeparableBSSRDF_BSDF bsdf_;

    public:

        SeparableBSSRDF_BSDFMaterial(const Intersection &pi, const SeparableBSSRDF *bssrdf) noexcept
            : bsdf_(pi, bssrdf)
        {
            
        }

        ShadingPoint GetShadingPoint(const Intersection &inct, [[maybe_unused]] Arena &arena) const override
        {
            ShadingPoint shd;
            shd.coordSys = inct.coordSys;
            shd.uv       = inct.uv;
            shd.bsdf     = &bsdf_;
            return shd;
        }
    };
}

SeparableBSSRDF::SeparableBSSRDF(const Intersection &po, Real eta) noexcept
    : BSSRDF(po), eta_(eta)
{
    
}

Spectrum SeparableBSSRDF::Eval(const Intersection &pi, [[maybe_unused]] bool star) const noexcept
{
    Real cosThetaI = Cos(pi.wr, pi.coordSys.ez);
    Real cosThetaO = Cos(po_.wr, po_.coordSys.ez);

    Real cI = 1 - 2 * FresnelMoment(eta_);

    auto ret =  Spectrum(1 - ComputeFresnelDielectric(1, eta_, cosThetaI)) *
                Spectrum(1 - ComputeFresnelDielectric(1, eta_, cosThetaO)) / (cI * PI);
    
    return ret;
}

Option<BSSRDF::SamplePiResult> SeparableBSSRDF::SamplePi(bool star, const Vec3 &sample, Arena &arena) const noexcept
{
    // 选择投影轴和采样通道

    auto [channel, sampleX] = AGZ::Math::DistributionTransform
        ::SampleExtractor<Real>::ExtractInteger<int>(sample.x, 0, SPECTRUM_CHANNEL_COUNT);

    // 构造投影坐标系

    CoordSystem projCoord;

    Real sampleY;
    if(sample.y < 0.5)
    {
        projCoord = po_.coordSys;
        sampleY = 2 * sample.y;
    }
    else if(sample.y < 0.75)
    {
        projCoord = CoordSystem(po_.coordSys.ey, po_.coordSys.ez, po_.coordSys.ex);
        sampleY = 4 * (sample.y - Real(0.5));
    }
    else
    {
        projCoord = CoordSystem(po_.coordSys.ez, po_.coordSys.ex, po_.coordSys.ey);
        sampleY = 4 * (sample.y - Real(0.75));
    }

    // 极坐标采样

    auto sr = SampleSr(channel, sampleX);
    Real rMax = SampleSr(channel, Real(0.996)).radius;
    if(sr.radius <= 0 || sr.radius > rMax)
        return None;
    Real phi = 2 * PI * sampleY;

    // 构造采样射线

    Real hl = Sqrt(Max(Real(0), rMax * rMax - sr.radius * sr.radius));
    Vec3 inctRayOri(sr.radius * Cos(phi), sr.radius * Sin(phi), hl);
    Real inctRayLength = 2 * hl;

    Ray inctRay(
        po_.pos + projCoord.Local2World(inctRayOri),
        -projCoord.ez,
        EPS, Max(EPS, inctRayLength));

    // 求采样射线与本物体的所有交点

    struct InctListNode
    {
        Intersection inct;
        InctListNode *next;
    } *inctListEntry = nullptr;
    int inctListLength = 0;

    for(;;)
    {
        if(inctRay.t0 >= inctRay.t1)
            break;

        Intersection nInct;
        if(!po_.entity->FindIntersection(inctRay, &nInct))
            break;
        AGZ_ASSERT((nInct.pos - inctRay.o).Length() <= 2 * hl + EPS);
        if(nInct.material == po_.material)
        {
            auto nNode = arena.Create<InctListNode>();
            nNode->inct = nInct;
            nNode->next = inctListEntry;
            inctListEntry = nNode;
            ++inctListLength;
        }
        inctRay.o = inctRay.At(nInct.t + EPS);
        inctRay.t1 -= nInct.t + EPS;
    }

    if(!inctListLength)
        return None;

    // 从交点中选取一个

    auto [inctIdx, sampleZ] = AGZ::Math::DistributionTransform::
        SampleExtractor<Real>::ExtractInteger<int>(sample.z, 0, inctListLength);
    for(int i = 0; i < inctIdx; ++i)
    {
        AGZ_ASSERT(inctListEntry);
        inctListEntry = inctListEntry->next;
        AGZ_ASSERT(inctListEntry);
    }

    if(inctListEntry->inct.material != po_.material)
        return None;

    Real pdfRadius = SamplePiPDF(inctListEntry->inct, star);

    SamplePiResult ret;
    ret.pi          = inctListEntry->inct;
    ret.pi.wr       = ret.pi.coordSys.ez;
    ret.pi.material = arena.Create<SeparableBSSRDF_BSDFMaterial>(ret.pi, this);
    ret.coef        = Sr((ret.pi.pos - po_.pos).Length());
    ret.pdf         = pdfRadius / (2 * PI) / inctListLength;

    if(ret.pdf < 0.01)
        return None;

    AGZ_ASSERT(sr.radius > 0);
    AGZ_ASSERT(ret.pdf > 0);
    AGZ_ASSERT((ret.pi.pos - po_.pos).Length() + EPS > sr.radius);

    return ret;
}

Real SeparableBSSRDF::SamplePiPDF(const Intersection &pi, [[maybe_unused]] bool star) const noexcept
{
    Vec3 ld = po_.coordSys.World2Local(po_.pos - pi.pos);
    Vec3 ln = po_.coordSys.World2Local(pi.coordSys.ez);
    Real rProj[] = { ld.yz().Length(), ld.zx().Length(), ld.xy().Length() };

    constexpr Real AXIS_PDF[] = { Real(0.25), Real(0.25), Real(0.5) };
    constexpr Real CHANNEL_PDF = Real(1) / SPECTRUM_CHANNEL_COUNT;

    Real ret = 0;
    for(int axisIdx = 0; axisIdx < 3; ++axisIdx)
    {
        for(int chIdx = 0; chIdx < SPECTRUM_CHANNEL_COUNT; ++chIdx)
            ret += Abs(ln[axisIdx]) * SampleSrPDF(chIdx, rProj[axisIdx]) * AXIS_PDF[axisIdx];
    }

    return ret * CHANNEL_PDF;
}

} // namespace Atrc
