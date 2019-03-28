#pragma once

#include <mutex>

#include <AGZUtils/Utils/Time.h>
#include <Atrc/Core/Core/Reporter.h>
#include <Atrc/Editor/GL.h>

class FilmRTReporter : public Atrc::Reporter, public AGZ::Uncopiable
{
public:

    FilmRTReporter();

    void Start() override;

    void End() override;

    void Report(const Atrc::Film &film, std::optional<Atrc::Real> percent) override;

    void Message(std::string_view msg) override;

    template<typename Func>
    void ConsumeNewData(Func &&func)
    {
        std::lock_guard<std::mutex> lk(mut_);
        if(newData_)
            func(img_);
        newData_ = false;
    }

    float GetPercent() const noexcept
    {
        return percent_;
    }

private:

    bool newData_;
    std::atomic<Atrc::Real> percent_;

    AGZ::Texture2D<Vec4f> img_;
    std::mutex mut_;

    AGZ::Clock clock_;
};
