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

    struct SamplePiWiResult
    {
        Intersection pi;
        Spectrum coef;
        Real pdf;
    };

    virtual SamplePiWiResult SamplePiWi(const Scene &scene, const Vec4 &sample, bool star) const noexcept = 0;

    virtual Real SamplePiWiPDF(const Scene &scene, const Intersection &pi, bool star) const noexcept = 0;
};

} // namespace Atrc
