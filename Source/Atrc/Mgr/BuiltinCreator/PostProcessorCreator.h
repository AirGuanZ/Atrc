#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{
    
void RegisterBuiltinPostProcessorCreators(Context &context);

/*
    type = FlipImage
*/
class FlipImageCreator : public Creator<PostProcessor>
{
public:

    Str8 GetTypeName() const override { return "FlipImage"; }

    PostProcessor *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = GammaCorrection

    gamma = Real
*/
class GammaCorrectionCreator : public Creator<PostProcessor>
{
public:

    Str8 GetTypeName() const override { return "GammaCorrection"; }

    PostProcessor *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
