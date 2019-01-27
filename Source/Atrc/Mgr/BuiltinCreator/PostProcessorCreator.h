#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{
    
void RegisterBuiltinPostProcessorCreators(Context &context);

/*
    type = ACESFilm
*/
class ACESFilmCreator : public Creator<PostProcessor>
{
public:

    std::string GetTypeName() const override { return "ACESFilm"; }

    PostProcessor *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = FlipImage
*/
class FlipImageCreator : public Creator<PostProcessor>
{
public:

    std::string GetTypeName() const override { return "FlipImage"; }

    PostProcessor *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = GammaCorrection

    gamma = Real
*/
class GammaCorrectionCreator : public Creator<PostProcessor>
{
public:

    std::string GetTypeName() const override { return "GammaCorrection"; }

    PostProcessor *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = NativeToneMapping
*/
class NativeToneMappingCreator : public Creator<PostProcessor>
{
public:

    std::string GetTypeName() const override { return "NativeToneMapping"; }

    PostProcessor *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

class SaveAsHDRCreator : public Creator<PostProcessor>
{
public:

    std::string GetTypeName() const override { return "SaveAsHDR"; }

    PostProcessor *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
