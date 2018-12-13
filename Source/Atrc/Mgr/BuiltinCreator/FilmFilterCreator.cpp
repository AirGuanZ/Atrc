#include <Atrc/Lib/FilmFilter/BoxFilter.h>
#include <Atrc/Mgr/BuiltinCreator/FilmFilterCreator.h>

namespace Atrc::Mgr
{

const FilmFilter *BoxFilterCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena)
{
    ATRC_MGR_TRY
    {
        auto sidelen = group["sidelen"].Parse<Real>();
        if(sidelen <= 0)
            throw MgrErr("Invalid sidelen value");
        return arena.Create<BoxFilter>(Vec2(sidelen / 2));
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating box filter: " + group.ToString())
}

} // namespace Atrc::Mgr
