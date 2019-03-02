#pragma once

#include <Atrc/Lib/Core/ResourceData.h>
#include <Atrc/Lib/Core/TFilm.h>

namespace Atrc
{

class BoxFilterData : public ResourceData<FilmFilter>
{
    Vec2 radius_;

public:

    static const std::string &GetTypeName() { static const std::string RET = "Box"; return RET; }

    std::string Serialize() const override;

    void Deserialize(const AGZ::ConfigGroup &param) override;
};

using BoxFilterCreator = ResourceData2Creator<BoxFilterData>;

class BoxFilter : public FilmFilter
{
    Vec2 radius_;

public:

    explicit BoxFilter(const Vec2 &radius) noexcept;

    Real Eval(Real relX, Real relY) const noexcept override;
};

} // namespace Atrc
