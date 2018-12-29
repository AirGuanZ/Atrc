#include <Atrc/Lib/Material/BSSRDF/SeparableBSSRDF.h>
#include <Atrc/Lib/Material/Utility/Fresnel.h>

namespace Atrc
{

namespace
{
    Real FresnelMoment(Real eta) {
        Real eta2 = eta * eta, eta3 = eta2 * eta, eta4 = eta3 * eta,
            eta5 = eta4 * eta;
        if(eta < 1)
        {
            return Real(0.45966) - Real(1.73965) * eta + Real(3.37668) * eta2 - Real(3.904945) * eta3 +
                   Real(2.49277) * eta4 - Real(0.68441) * eta5;
        }
        return Real(-4.61686) + Real(11.1136) * eta - Real(10.4646) * eta2 + Real(5.11455) * eta3 -
               Real(1.27198) * eta4 + Real(0.12746) * eta5;
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

BSSRDF::SamplePiResult SeparableBSSRDF::SamplePi(const Scene &scene, bool star, const Vec4 &sample) const noexcept
{
    SamplePiResult ret;
}

} // namespace Atrc
