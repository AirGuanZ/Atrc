#include <Atrc/Lib/Core/Ray.h>
#include <Atrc/Lib/Core/Scene.h>
#include <Atrc/Lib/Material/BSSRDF/SeparableBSSRDF.h>
#include <Atrc/Lib/Material/Utility/Fresnel.h>

namespace Atrc
{

namespace
{
    Real FresnelMoment(Real eta) {
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

    return Sr((pi.pos - po_.pos).Length()) * 
           (1 - ComputeFresnelDielectric(1, eta_, cosThetaI)) *
           (1 - ComputeFresnelDielectric(1, eta_, cosThetaO)) *
           InvPI2 / (cI * cO);
}

Option<BSSRDF::SamplePiResult> SeparableBSSRDF::SamplePi(
    const Scene &scene, bool star, const Vec4 &sample, Arena &arena) const noexcept
{
    // 选择投影轴和采样通道

    auto [channel, sXT] = AGZ::Math::DistributionTransform
        ::SampleExtractor<Real>::ExtractInteger<int>(sample.x, 0, SPECTRUM_CHANNEL_COUNT);
    auto[projAxis, sampleX] = AGZ::Math::DistributionTransform
        ::SampleExtractor<Real>::ExtractInteger<int>(sXT, 0, 3);
    Real channelPDF = Real(1) / SPECTRUM_CHANNEL_COUNT;

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


}

} // namespace Atrc
