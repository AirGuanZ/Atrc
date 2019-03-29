#pragma once

#include <Atrc/Editor/FilmFilter/FilmFilter.h>

class Box : public IFilmFilter
{
    float sidelen_ = 1;

public:

    using IFilmFilter::IFilmFilter;

    std::string Save() const override;

    void Load(const AGZ::ConfigGroup &params) override;

    std::string Export() const override;

    void Display() override;

    bool IsMultiline() const noexcept override;
};

class BoxCreator : public IFilmFilterCreator
{
public:

    BoxCreator() : IFilmFilterCreator("Box") { }

    std::shared_ptr<IFilmFilter> Create() const override;
};
