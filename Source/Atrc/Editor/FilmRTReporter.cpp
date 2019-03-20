#include <Atrc/Editor/FilmRTReporter.h>
#include <Atrc/Editor/Global.h>
#include <Atrc/Lib/Core/TFilm.h>

FilmRTReporter::FilmRTReporter()
    : newData_(false), percent_(0)
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

    auto &values = film.GetValues();
    auto &weights = film.GetWeights();
    img_ = AGZ::Texture2D<Vec4f>(weights.GetWidth(), weights.GetHeight());
    for(uint32_t y = 0; y < img_.GetHeight(); ++y)
    {
        for(uint32_t x = 0; x < img_.GetWidth(); ++x)
            img_(x, y) = weights(x, y) > 0 ? Vec4f(values(x, y) / weights(x, y), 1) : Vec4f();
    }

    newData_ = true;
    if(percent)
        percent_ = *percent;
}

void FilmRTReporter::Message(std::string_view msg)
{
    Global::ShowNormalMessage(std::string(msg));
}
