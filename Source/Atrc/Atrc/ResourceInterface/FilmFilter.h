#pragma once

#include <Atrc/Atrc/ResourceInterface/ResourceInstance.h>

class FilmFilterInstance : public ResourceInstance
{
public:

    using ResourceInstance ::ResourceInstance;
};

using FilmFilterCreator = ResourceInstanceCreator<FilmFilterInstance>;
using FilmFilterCreatorManager = ResourceInstanceCreatorManager<FilmFilterInstance>;

template<typename TFilmFilterInstance>
using FilmFilter2Creaotr = ResourceInstance2Creator<FilmFilterInstance, TFilmFilterInstance>;

template<typename TFilmFilterCore>
using WidgetCore2FilmFilterInstance = WidgetCore2ResourceInstance<FilmFilterInstance, TFilmFilterCore>;

void RegisterBuiltinFilmFilterCreators(FilmFilterCreatorManager &mgr);
