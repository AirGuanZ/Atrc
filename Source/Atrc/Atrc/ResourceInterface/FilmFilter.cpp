#include <Atrc/Atrc/ResourceInterface/FilmFilter.h>

const std::map<std::string, const FilmFilterInstanceCreator*> &FilmFilterInstanceCreator::GetAllCreators()
{
    static const std::map<std::string, const FilmFilterInstanceCreator*> ret;
    return ret;
}
