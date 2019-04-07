#pragma once

#include <Atrc/Editor/FilmFilter/FilmFilter.h>

class Gaussian : public IFilmFilter
{
    float radius_ = 0.7f;
    float alpha_ = 2;

public:

    using IFilmFilter::IFilmFilter;

    std::string Save() const override;

    void Load(const AGZ::ConfigGroup &params) override;

    std::string Export() const override;

    void Display() override;

    bool IsMultiline() const noexcept override;
};

DEFINE_DEFAULT_RESOURCE_CREATOR(IFilmFilterCreator, Gaussian, "Gaussian");
