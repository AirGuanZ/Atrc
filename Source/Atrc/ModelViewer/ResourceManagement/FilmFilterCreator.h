#pragma once

#include <Atrc/ModelViewer/ResourceManagement/ResourceManager.h>

void RegisterFilmFilterCreators(ResourceManager &rscMgr);

class BoxFilmterCreator : public FilmFilterCreator
{
public:

    BoxFilmterCreator() : FilmFilterCreator("box") { }

    std::shared_ptr<FilmFilterInstance> Create(std::string name) const override;
};
