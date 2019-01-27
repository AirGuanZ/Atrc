#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

void RegisterBuiltinFilmFilterCreators(Context &context);

/*
    type = Box

    sidelen = Real
*/
class BoxFilterCreator : public Creator<FilmFilter>
{
public:

    std::string GetTypeName() const override { return "Box"; }

    FilmFilter *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

class GaussianFilterCreator : public Creator<FilmFilter>
{
public:

    std::string GetTypeName() const override { return "Gaussian"; }

    FilmFilter *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
