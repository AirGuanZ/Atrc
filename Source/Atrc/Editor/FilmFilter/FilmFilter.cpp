#include <Atrc/Editor/FilmFilter/Box.h>
#include <Atrc/Editor/FilmFilter/Gaussian.h>
#include <Atrc/Editor/FilmFilter/FilmFilter.h>

void RegisterBuiltinFilmFilterCreators(FilmFilterFactory &factory)
{
    static const BoxCreator iBoxCreator;
    static const GaussianCreator iGaussianCreator;
    factory.AddCreator(&iBoxCreator);
    factory.AddCreator(&iGaussianCreator);
}
