#pragma once

#include <memory>

#include <AGZUtils/Utils/Config.h>
#include <Atrc/Editor/ResourceInstance/ResourceInstance.h>

namespace Atrc::Editor
{

class IFresnel : public ResourceCommon<IFresnel>
{
public:

    using ResourceCommon<IFresnel>::ResourceCommon;

    static const char *StrID() noexcept { return "fresnel"; }

    virtual std::string Save() const = 0;

    virtual void Load(const AGZ::ConfigGroup &params) = 0;

    virtual std::string Export() const = 0;

    virtual void Display() = 0;

    virtual bool IsMultiline() const noexcept = 0;
};

DEFINE_DEFAULT_RESOURCE_CREATOR_INTERFACE(IFresnel);

void RegisterBuiltinFresnelCreators(FresnelFactory &factory);

}; // namespace Atrc::Editor
