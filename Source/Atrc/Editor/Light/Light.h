#pragma once

#include <memory>

#include <AGZUtils/Utils/Config.h>
#include <Atrc/Editor/ResourceInstance/ResourceInstance.h>

namespace Atrc::Editor
{

class ILight : public ResourceCommon<ILight>
{
public:

    using ResourceCommon<ILight>::ResourceCommon;

    static const char *StrID() noexcept { return "light"; }

    virtual std::string Save(const std::filesystem::path &relPath) const = 0;

    virtual void Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath) = 0;

    virtual std::string Export(const std::filesystem::path &path) const = 0;

    virtual void Display() = 0;

    virtual bool IsMultiline() const noexcept = 0;
};

DEFINE_DEFAULT_RESOURCE_CREATOR_INTERFACE(ILight);

void RegisterBuiltinLightCreators(LightFactory &factory);

}; // namespace Atrc::Editor
