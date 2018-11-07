#pragma once

#include "../Common.h"

using MediumCreator = ObjectCreator<Atrc::Medium>;
using MediumManager = ObjectManager<Atrc::Medium>;

// sigmaA = Spectrum
// sigmaS = Spectrum
// le     = Spectrum
// g      = Real
class HomongeneousMediumCreator : public MediumCreator, public AGZ::Singleton<HomongeneousMediumCreator>
{
public:

    Str8 GetName() const override { return "Homogeneous"; }

    Atrc::Medium* Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};
