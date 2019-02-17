#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>

void RegisterFilmFilterCreators(ResourceManager &rscMgr);

class BoxFilterCreator : public FilmFilterCreator
{
public:

    BoxFilterCreator() : FilmFilterCreator("Box") { }

    std::shared_ptr<FilmFilterInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class GaussianFilterCreator : public FilmFilterCreator
{
public:

    GaussianFilterCreator() : FilmFilterCreator("Gaussian") { }

    std::shared_ptr<FilmFilterInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};
