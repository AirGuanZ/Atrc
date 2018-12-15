#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

void RegisterBuiltinReportCreators(Context &context);
    
class DefaultReporterCreator : public Creator<Reporter>
{
public:

    Str8 GetTypeName() const override { return "Default"; }

    Reporter *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
