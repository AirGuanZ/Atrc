#pragma once

#include <Atrc/Lib/Material/BSSRDF/SeparableBSSRDF.h>

namespace Atrc
{
    
class NormalizedDiffusionBSSRDF : public SeparableBSSRDF
{
    Spectrum d_;

protected:

    NormalizedDiffusionBSSRDF(const Intersection &inct, Real eta, const Spectrum &mfp) noexcept;

    Spectrum Sr(Real distance) const noexcept override;

    SampleSrResult SampleSr(int channel, Real sample) const noexcept override;

    Real SampleSrPDF(int channel, Real distance) const noexcept override;
};

} // namespace Atrc
