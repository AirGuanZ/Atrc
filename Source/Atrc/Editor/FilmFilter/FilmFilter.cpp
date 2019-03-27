#include <Atrc/Editor/FilmFilter/Box.h>
#include <Atrc/Editor/FilmFilter/FilmFilter.h>

void RegisterBuiltinFilmFilterCreators(ResourceFactory<IFilmFilterCreator> &factory)
{
    static const BoxCreator iBoxCreator;
    factory.AddCreator(&iBoxCreator);
}
