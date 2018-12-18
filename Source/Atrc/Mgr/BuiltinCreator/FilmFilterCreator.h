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

    Str8 GetTypeName() const override { return "Box"; }

    FilmFilter *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
