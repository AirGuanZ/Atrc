#pragma once

#include <mutex>

#include <Atrc/Lib/Core/Reporter.h>

#include <Atrc/Editor/GL.h>

class FilmRTReporter : public Atrc::Reporter, public AGZ::Uncopiable
{
public:

    FilmRTReporter(size_t width, size_t height);

    void Start() override;

    void End() override;

    void Report(const Atrc::Film &film, std::optional<Atrc::Real> percent) override;

    void Message(std::string_view msg) override;

    template<typename Func>
    void ProcessTexture(Func &&func)
    {
        std::lock_guard<std::mutex> lk(mut_);
        func(tex_);
    }

    void StartShowImage() noexcept { showImage_ = true; }
    void EndShowImage() noexcept { showImage_ = false; }
    bool IsShowingImage() const noexcept { return showImage_; }

private:

    bool showImage_;

    GL::Texture2D tex_;
    std::mutex mut_;
};
