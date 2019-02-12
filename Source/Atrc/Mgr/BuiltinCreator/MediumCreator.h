#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

void RegisterBuiltinMediumCreators(Context &context);

/*
    type = Homogeneous

    sigmaA = Spectrum
    sigmaS = Spectrum
    le     = Spectrum
    g      = Real
*/
class HomogeneousMediumCreator : public Creator<Medium>
{
public:

    std::string GetTypeName() const override { return "Homogeneous"; }

    Medium *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
