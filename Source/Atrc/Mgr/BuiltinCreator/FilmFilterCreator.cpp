#include <Atrc/Core/FilmFilter/BoxFilter.h>
#include <Atrc/Core/FilmFilter/GaussianFilter.h>
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
    AGZ_HIERARCHY_TRY
    {
        Real sidelen = group["sidelen"].Parse<Real>();
        if(sidelen <= 0)
            throw std::runtime_error("Invalid sidelen value");
        return arena.Create<BoxFilter>(Vec2(sidelen / 2));
    }
    AGZ_HIERARCHY_WRAP("In creating box filter: " + group.ToString())
}

FilmFilter *GaussianFilterCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        Real radius = group["radius"].Parse<Real>();
        Real alpha = group["alpha"].Parse<Real>();

        if(radius <= 0)
            throw std::runtime_error("Invalid radius value");

        return arena.Create<GaussianFilter>(Vec2(radius), alpha);
    }
    AGZ_HIERARCHY_WRAP("In creating gaussian filter")
}

} // namespace Atrc::Mgr
