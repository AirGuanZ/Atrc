#include <Atrc/Atrc/FilmFilter/FilmFilter.h>

#include <Atrc/Atrc/FilmFilter/Box.h>
#include <Atrc/Atrc/FilmFilter/Gaussian.h>

void RegisterFilmFilterCreators(ResourceCreatorManager<FilmFilterInstance> &mgr)
{
    static const Core2FilmFilterCreator<BoxCore> iBoxCreator;
    static const Core2FilmFilterCreator<GaussianCore> iGaussianCreator;
    mgr.AddCreator(&iBoxCreator);
    mgr.AddCreator(&iGaussianCreator);
}
