#pragma once

#include <Atrc/Lib/Core/ResourceData.h>
#include <Atrc/Lib/Core/TFilm.h>

namespace Atrc
{

class BoxFilterData : public ResourceData<FilmFilter>
{
    Real sidelen_ = 1;

public:

    static const std::string &GetTypeName() { static const std::string RET = "Box"; return RET; }

    std::string Serialize() const override;

    void Deserialize(const AGZ::ConfigGroup &param) override;

    FilmFilter *CreateResource(Arena &arena) const override;
};

} // namespace Atrc
