#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

/*
    type = Box

    sidelen = Real
*/
class BoxFilterCreator : public Creator<FilmFilter>
{
public:

    Str8 GetTypeName() const override { return "Box"; }

    const FilmFilter *Create(const ConfigGroup &group, Context &context, Arena &arena) override;
};

} // namespace Atrc::Mgr
