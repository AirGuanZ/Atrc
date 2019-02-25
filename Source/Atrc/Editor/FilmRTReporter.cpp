#include <Atrc/Editor/FilmRTReporter.h>
#include <Atrc/Editor/Global.h>
#include <Atrc/Lib/Core/TFilm.h>

FilmRTReporter::FilmRTReporter()
    : newData_(false)
{

}

void FilmRTReporter::Start()
{
    newData_ = false;
}

void FilmRTReporter::End()
{
    
}

void FilmRTReporter::Report(const Atrc::Film &film, std::optional<Atrc::Real> percent)
{
    std::lock_guard<std::mutex> lk(mut_);
    img_ = film.GetImage().Map([](auto &v)
    {
        return v.Map(&AGZ::TypeOpr::StaticCaster<float, Atrc::Real>);
    });
    newData_ = true;
    if(percent)
        Global::ShowNormalMessage("percent: " + std::to_string(*percent));
}

void FilmRTReporter::Message(std::string_view msg)
{
    Global::ShowNormalMessage(std::string(msg));
}
