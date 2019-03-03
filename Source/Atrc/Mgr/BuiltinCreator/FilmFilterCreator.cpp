#include <Atrc/Lib/FilmFilter/BoxFilter.h>
#include <Atrc/Lib/FilmFilter/GaussianFilter.h>
#include <Atrc/Mgr/BuiltinCreator/FilmFilterCreator.h>

namespace Atrc::Mgr
{

void RegisterBuiltinFilmFilterCreators(Context &context)
{
    static const BoxFilterCreator boxFilterCreator;
    static const GaussianFilterCreator gaussianFilterCreator;
    context.AddCreator(&gaussianFilterCreator);
    context.AddCreator(&boxFilterCreator);
}

FilmFilter *BoxFilterCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        BoxFilterData data;
        data.Deserialize(group);
        return data.CreateResource(arena);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating box filter: " + group.ToString())
}

FilmFilter *GaussianFilterCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        GaussianFilterData data;
        data.Deserialize(group);
        return data.CreateResource(arena);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating gaussian filter")
}

} // namespace Atrc::Mgr
