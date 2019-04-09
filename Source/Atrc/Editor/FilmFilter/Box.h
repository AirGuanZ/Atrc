#pragma once

#include <Atrc/Editor/FilmFilter/FilmFilter.h>

namespace Atrc::Editor
{

class Box : public ResourceCommonImpl<IFilmFilter, Box>
{
    float sidelen_ = 1;

public:

    using ResourceCommonImpl<IFilmFilter, Box>::ResourceCommonImpl;

    std::string Save() const override;

    void Load(const AGZ::ConfigGroup &params) override;

    std::string Export() const override;

    void Display() override;

    bool IsMultiline() const noexcept override;
};

DEFINE_DEFAULT_RESOURCE_CREATOR(IFilmFilterCreator, Box, "Box");

}; // namespace Atrc::Editor
