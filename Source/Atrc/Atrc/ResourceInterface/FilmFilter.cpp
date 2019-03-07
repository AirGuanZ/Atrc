#include <Atrc/Atrc/FilmFilter/Box.h>
#include <Atrc/Atrc/ResourceInterface/FilmFilter.h>

void RegisterBuiltinFilmFilterCreators(FilmFilterCreatorManager &mgr)
{
    static const FilmFilter2Creaotr<Box> boxCreator;
    mgr.Add(&boxCreator);
}
