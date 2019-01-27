#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

void RegisterBuiltinReportCreators(Context &context);
    
class DefaultReporterCreator : public Creator<Reporter>
{
public:

    std::string GetTypeName() const override { return "Default"; }

    Reporter *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
