#include <Atrc/Lib/Core/Entity.h>
#include <Atrc/Lib/Core/Material.h>
#include <Atrc/Lib/Core/Ray.h>
#include <Atrc/Lib/Material/BSSRDF/SeparableBSSRDF.h>
#include <Atrc/Lib/Material/Utility/Fresnel.h>

namespace Atrc
{

namespace
{
    Real FresnelMoment(Real eta)
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
        const BSSRDF *bssrdf_;

    public:

        SeparableBSSRDF_BSDF(const Intersection &pi, const BSSRDF *bssrdf) noexcept
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
            [[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &wo,
            BSDFType type, bool star) const noexcept override
        {
            if(!((type & BSDF_TRANSMISSION) && (type & (BSDF_DIFFUSE | BSDF_GLOSSY))))
                return Spectrum();
            return bssrdf_->Eval(pi_, star);
        }

        Option<SampleWiResult> SampleWi(
            const CoordSystem &shd, const CoordSystem &geo,
            const Vec3 &wo, BSDFType type, bool star, const Vec2 &sample) const noexcept override
        {
            if(!((type & BSDF_TRANSMISSION) && (type & (BSDF_DIFFUSE | BSDF_GLOSSY))))
                return None;

            auto[sam, pdf] = AGZ::Math::DistributionTransform
                ::ZWeightedOnUnitHemisphere<Real>::Transform(sample);
            if(!pdf)
                return None;

            SampleWiResult ret;
            ret.coef    = Eval(shd, geo, sam, wo, BSDF_ALL, star);
            ret.pdf     = pdf;
            ret.type    = BSDFType(BSDF_TRANSMISSION | BSDF_GLOSSY);
            ret.wi      = sam;
            ret.isDelta = false;

            return ret;
        }

        Real SampleWiPDF(
            [[maybe_unused]] const CoordSystem &shd, [[maybe_unused]] const CoordSystem &geo,
            const Vec3 &wi, const Vec3 &wo, BSDFType type, [[maybe_unused]] bool star) const noexcept override
        {
            if(!((type & BSDF_TRANSMISSION) && (type & (BSDF_DIFFUSE | BSDF_GLOSSY))))
                return 0;
            if(wi.z <= 0 || wo.z <= 0)
                return 0;
            return AGZ::Math::DistributionTransform::ZWeightedOnUnitHemisphere<Real>::PDF(wi.Normalize());
        }
    };

    class SeparableBSSRDF_BSDFMaterial : public Material
    {
        SeparableBSSRDF_BSDF bsdf_;

    public:

        SeparableBSSRDF_BSDFMaterial(const Intersection &pi, const BSSRDF *bssrdf) noexcept
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

Spectrum SeparableBSSRDF::Eval(const Intersection &pi, bool star) const noexcept
{
    constexpr Real InvPI2 = 1 / (PI * PI);

    Real cosThetaI = Cos(pi.wr, pi.coordSys.ez);
    Real cosThetaO = Cos(po_.wr, po_.coordSys.ez);

    Real cI = 1 - 2 * FresnelMoment(eta_);
    Real cO = 1 - 2 * FresnelMoment(1 / eta_);

    auto ret =  Sr((pi.pos - po_.pos).Length()) * 
                (1 - ComputeFresnelDielectric(1, eta_, cosThetaI)) *
                (1 - ComputeFresnelDielectric(1, eta_, cosThetaO)) *
                InvPI2 / (cI * cO);
    if(!star)
        ret *= eta_ * eta_;
    return ret;
}

Option<BSSRDF::SamplePiResult> SeparableBSSRDF::SamplePi(bool star, const Vec3 &sample, Arena &arena) const noexcept
{
    // 选择投影轴和采样通道

    auto [channel, sXT] = AGZ::Math::DistributionTransform
        ::SampleExtractor<Real>::ExtractInteger<int>(sample.x, 0, SPECTRUM_CHANNEL_COUNT);
    auto[projAxis, sampleX] = AGZ::Math::DistributionTransform
        ::SampleExtractor<Real>::ExtractInteger<int>(sXT, 0, 3);

    // 极座标采样

    auto sr = SampleSr(channel, sampleX);
    Real rMax = SampleSr(channel, Real(0.999)).radius;
    if(sr.radius < 0 || sr.radius > rMax)
        return None;
    Real phi = 2 * PI * sample.y;

    // 构造投影坐标系

    CoordSystem projCoord;
    switch(projAxis)
    {
    case 0:
        projCoord = po_.coordSys;
        break;
    case 1:
        projCoord = CoordSystem(po_.coordSys.ey, po_.coordSys.ez, po_.coordSys.ex);
        break;
    case 2:
        projCoord = CoordSystem(po_.coordSys.ez, po_.coordSys.ex, po_.coordSys.ey);
        break;
    default:
        AGZ::Unreachable();
    }

    // 构造采样射线

    Real hl = Sqrt(Max(Real(0), rMax * rMax - sr.radius * sr.radius));
    Vec3 inctRayOri(sr.radius * Cos(phi), sr.radius * Sin(phi), hl);
    Vec3 inctRayDir = -Vec3::UNIT_Z();
    Real inctRayLength = 2 * hl;

    Ray inctRay(
        po_.pos + projCoord.Local2World(inctRayOri),
        projCoord.Local2World(inctRayDir).Normalize(),
        EPS, inctRayLength);

    // 求采样射线与本物体的所有交点

    struct InctListNode
    {
        Intersection inct;
        InctListNode *next;
    } *inctListEntry = nullptr;
    int inctListLength = 0;

    for(;;)
    {
        Intersection nInct;
        if(!po_.entity->FindIntersection(inctRay, &nInct))
            break;
        auto nNode = arena.Create<InctListNode>();
        nNode->inct = nInct;
        nNode->next = inctListEntry;
        inctListEntry = nNode;
        ++inctListLength;

        inctRay.t0 = nInct.t + EPS;
        if(inctRay.t0 >= inctRay.t1)
            break;
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

    Real pdf = SamplePiPDF(inctListEntry->inct, star) / inctListLength;

    SamplePiResult ret;
    ret.pi          = inctListEntry->inct;
    ret.pi.wr       = ret.pi.coordSys.ez;
    ret.pi.material = arena.Create<SeparableBSSRDF_BSDFMaterial>(ret.pi, this);
    ret.coef        = Spectrum(1);
    ret.pdf         = pdf;

    return ret;
}

Real SeparableBSSRDF::SamplePiPDF(const Intersection &pi, [[maybe_unused]] bool star) const noexcept
{
    Vec3 ld = po_.coordSys.World2Local(po_.pos - pi.pos);
    Vec3 lni = po_.coordSys.World2Local(pi.coordSys.ez);
    Real rProj[] = { ld.yz().Length(), ld.zx().Length(), ld.xy().Length() };

    constexpr Real AXIS_PDF = Real(1) / 3;
    constexpr Real CHANNEL_PDF = Real(1) / SPECTRUM_CHANNEL_COUNT;

    Real ret = 0;
    for(int axisIdx = 0; axisIdx < 3; ++axisIdx)
    {
        for(int chIdx = 0; chIdx < SPECTRUM_CHANNEL_COUNT; ++chIdx)
            ret += Abs(lni[axisIdx]) * SampleSrPDF(chIdx, rProj[axisIdx]) * AXIS_PDF * CHANNEL_PDF;
    }

    return ret;
}

} // namespace Atrc
