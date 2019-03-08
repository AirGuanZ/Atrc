#include <Atrc/Atrc/FilmFilter/Box.h>
#include <Atrc/Atrc/FilmFilter/Gaussian.h>
#include <Atrc/Atrc/ResourceInterface/FilmFilter.h>

void RegisterBuiltinFilmFilterCreators(FilmFilterCreatorManager &mgr)
{
    static const FilmFilter2Creaotr<Box> boxCreator;
    static const FilmFilter2Creaotr<Gaussian> gaussianCreator;
    mgr.Add(&boxCreator);
    mgr.Add(&gaussianCreator);
}
