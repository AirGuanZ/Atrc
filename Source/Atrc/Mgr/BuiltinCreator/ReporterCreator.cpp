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
    ATRC_MGR_TRY
    {
        return arena.Create<DefaultReporter>();
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating default reporter: " + group.ToString())
}

} // namespace Atrc::Mgr
