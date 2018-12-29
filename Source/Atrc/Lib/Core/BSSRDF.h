#pragma once

#include <Atrc/Lib/Core/SurfacePoint.h>

namespace Atrc
{
    
class BSSRDF
{
protected:

    const Intersection &po_;

public:

    explicit BSSRDF(const Intersection &po) noexcept : po_(po) { }

    virtual ~BSSRDF() = default;

    virtual Spectrum Eval(const Intersection &pi, bool star) const noexcept = 0;

    struct SamplePiResult
    {
        Intersection pi;
        Spectrum coef;
        Real pdf;
    };

    virtual SamplePiResult SamplePi(const Scene &scene, bool star, const Vec4 &sample) const noexcept = 0;

    virtual Real SamplePiPDF(const Scene &scene, const Intersection &pi, bool star) const noexcept = 0;
};

} // namespace Atrc
