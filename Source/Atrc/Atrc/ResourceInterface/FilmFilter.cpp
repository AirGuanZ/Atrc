#include <Atrc/Atrc/FilmFilter/Box.h>
#include <Atrc/Atrc/ResourceInterface/FilmFilter.h>

const std::map<std::string, const FilmFilterInstanceCreator*> &FilmFilterInstanceCreator::GetAllCreators()
{
    static const FilmFilterInstance2Creator<Box> boxCreator;

    static const std::map<std::string, const FilmFilterInstanceCreator*> ret =
    {
        { boxCreator.GetName(), &boxCreator }
    };

    return ret;
}
