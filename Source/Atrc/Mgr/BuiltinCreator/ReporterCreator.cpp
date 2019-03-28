#include <Atrc/Lib/Reporter/DefaultReporter.h>
#include <Atrc/Mgr/BuiltinCreator/ReporterCreator.h>

namespace Atrc::Mgr
{
    
void RegisterBuiltinReportCreators(Context &context)
{
    static const DefaultReporterCreator defaultReporterCreator;
    context.AddCreator(&defaultReporterCreator);
}

Reporter *DefaultReporterCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        return arena.Create<DefaultReporter>();
    }
    AGZ_HIERARCHY_WRAP("In creating default reporter: " + group.ToString())
}

} // namespace Atrc::Mgr
