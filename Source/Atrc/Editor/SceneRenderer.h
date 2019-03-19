#pragma once

#include <memory>

#include <Atrc/Editor/FilmRTReporter.h>
#include <Atrc/Editor/GL.h>
#include <Atrc/Lib/Core/Scene.h>
#include <Atrc/Lib/Core/TFilm.h>
#include <Atrc/Mgr/Context.h>

class SceneRenderer : public AGZ::Uncopiable
{
    std::unique_ptr<Atrc::Mgr::Context> context_;
    
    Atrc::Renderer   *renderer_;
    Atrc::Sampler    *sampler_;
    Atrc::FilmFilter *filmFilter_;

    std::unique_ptr<Atrc::Scene> scene_;

    std::unique_ptr<FilmRTReporter> reporter_;
    std::unique_ptr<Atrc::Film> film_;

    Vec2i filmSize_;
    std::string outputFilename_;

    bool saved_;

public:

    SceneRenderer();

    bool Start(const AGZ::Config &config, std::string_view configPath);

    void Stop();

    bool IsRendering() const;

    float GetRenderingPercent() const noexcept; // assert reporter_

    bool IsCompleted() const;

    void Join();
	
	void Clear();

    template<typename Func>
    void ProcessImage(Func &&func)
    {
        if(reporter_)
            reporter_->ConsumeNewData(std::forward<Func>(func));
    }

    void CheckSaving();
};
