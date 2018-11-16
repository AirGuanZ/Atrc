#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

// See http://www.astro.umd.edu/~jph/HG_note.pdf
class HenyeyGreenstein : public PhaseFunction
{
    Vec3 wo;
    Real g;

public:

    explicit HenyeyGreenstein(const Vec3 &wo, Real g);

    PhaseFunctionSampleWiResult SampleWi() const override;
};

AGZ_NS_END(Atrc)
