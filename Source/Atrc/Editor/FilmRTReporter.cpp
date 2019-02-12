#include <Atrc/Lib/Core/TFilm.h>

#include <Atrc/Editor/FilmRTReporter.h>
#include <Atrc/Editor/Global.h>

FilmRTReporter::FilmRTReporter(size_t width, size_t height)
{
    tex_.InitializeHandle();
    tex_.InitializeFormat(1, width, height, GL_RGB8);
    showImage_ = false;
}

void FilmRTReporter::Start()
{
    showImage_ = true;
}

void FilmRTReporter::End()
{
    
}

void FilmRTReporter::Report(const Atrc::Film &film, std::optional<Atrc::Real> percent)
{
    std::lock_guard<std::mutex> lk(mut_);
    auto img = film.GetImage();
    tex_.ReinitializeData(img.GetWidth(), img.GetHeight(), img.RawData());
}

void FilmRTReporter::Message(std::string_view msg)
{
    Global::ShowNormalMessage(std::string(msg));
}
