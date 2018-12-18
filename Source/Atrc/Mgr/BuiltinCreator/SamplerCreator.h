#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

void RegisterBuiltinSamplerCreators(Context &context);

/*
    type = Native

    seed = int | null
    spp  = int
*/
class NativeSamplerCreator : public Creator<Sampler>
{
public:

    Str8 GetTypeName() const override { return "Native"; }

    Sampler *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
