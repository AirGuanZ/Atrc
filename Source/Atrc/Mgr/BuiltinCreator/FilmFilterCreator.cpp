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
        Real sidelen = group["sidelen"].Parse<Real>();
        if(sidelen <= 0)
            throw MgrErr("Invalid sidelen value");
        return arena.Create<BoxFilter>(Vec2(sidelen / 2));
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating box filter: " + group.ToString())
}

FilmFilter *GaussianFilterCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        Real radius = group["radius"].Parse<Real>();
        Real alpha = group["alpha"].Parse<Real>();

        if(radius <= 0)
            throw MgrErr("Invalid radius value");

        return arena.Create<GaussianFilter>(Vec2(radius), alpha);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating gaussian filter")
}

} // namespace Atrc::Mgr
