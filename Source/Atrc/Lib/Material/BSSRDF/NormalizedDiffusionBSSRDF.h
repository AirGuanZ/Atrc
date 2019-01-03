#pragma once

#include <Atrc/Lib/Material/BSSRDF/SeparableBSSRDF.h>

namespace Atrc
{
    
// See http://graphics.pixar.com/library/ApproxBSSRDF/paper.pdf
class NormalizedDiffusionBSSRDF : public SeparableBSSRDF
{
    Spectrum A_;
    Spectrum s_;
    Spectrum l_;
    Spectrum d_;

protected:

    NormalizedDiffusionBSSRDF(const Intersection &inct, Real eta, const Spectrum &A, const Spectrum &mfp) noexcept;

    Spectrum Sr(Real distance) const noexcept override;

    SampleSrResult SampleSr(int channel, Real sample) const noexcept override;

    Real SampleSrPDF(int channel, Real distance) const noexcept override;
};

} // namespace Atrc
