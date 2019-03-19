#include <Atrc/Editor/FilmRTReporter.h>
#include <Atrc/Editor/Global.h>
#include <Atrc/Lib/Core/TFilm.h>

FilmRTReporter::FilmRTReporter()
    : newData_(false)
{

}

void FilmRTReporter::Start()
{
    Global::ShowNormalMessage("start rendering...");
    newData_ = false;
    percent_ = 0;
    clock_.Restart();
}

void FilmRTReporter::End()
{
    Global::ShowNormalMessage("complete rendering...total time: " + std::to_string(clock_.Milliseconds() / 1000.0) + "s");
}

void FilmRTReporter::Report(const Atrc::Film &film, std::optional<Atrc::Real> percent)
{
    std::lock_guard<std::mutex> lk(mut_);
    img_ = film.GetImage().Map([](const Atrc::Spectrum &v)
    {
        return v.Map(&AGZ::TypeOpr::StaticCaster<float, Atrc::Real>);
    });
    newData_ = true;
    if(percent)
        percent_ = *percent;
}

void FilmRTReporter::Message(std::string_view msg)
{
    Global::ShowNormalMessage(std::string(msg));
}
