#pragma once

#include <Atrc/Lib/Core/BSSRDF.h>

namespace Atrc
{

class SeparableBSSRDF : public BSSRDF
{
protected:

    virtual Spectrum Sr(Real distance) const noexcept = 0;

    struct SampleSrResult
    {
        Spectrum coef;
        Real radius;
        Real pdf;
    };

    virtual SampleSrResult SampleSr(int channel, Real sample) const noexcept = 0;

    virtual Real SampleSrPDF(int channel, Real distance) const noexcept = 0;

    Real eta_;

public:

    SeparableBSSRDF(const Intersection &po, Real eta) noexcept;

    Spectrum Eval(const Intersection &pi, bool star) const noexcept override;

    Option<SamplePiResult> SamplePi(const Scene &scene, bool star, const Vec4 &sample, Arena &arena) const noexcept override;

    Real SamplePiPDF(const Scene &scene, const Intersection &pi, bool star) const noexcept override;
};

} // namespace Atrc
