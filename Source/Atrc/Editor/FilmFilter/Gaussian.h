#pragma once

#include <Atrc/Editor/FilmFilter/FilmFilter.h>

namespace Atrc::Editor
{

class Gaussian : public ResourceCommonImpl<IFilmFilter, Gaussian>
{
    float radius_ = 0.7f;
    float alpha_ = 2;

public:

    using ResourceCommonImpl<IFilmFilter, Gaussian>::ResourceCommonImpl;

    std::string Save() const override;

    void Load(const AGZ::ConfigGroup &params) override;

    std::string Export() const override;

    void Display() override;

    bool IsMultiline() const noexcept override;
};

DEFINE_DEFAULT_RESOURCE_CREATOR(IFilmFilterCreator, Gaussian, "Gaussian");

}; // namespace Atrc::Editor
