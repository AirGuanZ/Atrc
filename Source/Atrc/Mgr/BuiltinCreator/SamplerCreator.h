#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

/*
    type = Native

    seed = int | null
    spp  = int
*/
class NativeSamplerCreator : public Creator<Sampler>
{
public:

    Str8 GetTypeName() const override { return "Native"; }

    const Sampler *Create(const ConfigGroup &group, Context &context, Arena &arena) override;
};

} // namespace Atrc::Mgr
