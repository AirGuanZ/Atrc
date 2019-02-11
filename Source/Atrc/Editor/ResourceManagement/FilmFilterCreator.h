#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>

void RegisterFilmFilterCreators(ResourceManager &rscMgr);

class BoxFilterCreator : public FilmFilterCreator
{
public:

    BoxFilterCreator() : FilmFilterCreator("box") { }

    std::shared_ptr<FilmFilterInstance> Create(std::string name) const override;
};

class GaussianFilterCreator : public FilmFilterCreator
{
public:

    GaussianFilterCreator() : FilmFilterCreator("gaussian") { }

    std::shared_ptr<FilmFilterInstance> Create(std::string name) const override;
};
