#pragma once

#include <memory>

#include <Atrc/Editor/GL.h>
#include <Atrc/Lib/Core/Scene.h>
#include <Atrc/Mgr/Context.h>

class SceneRenderer
{
    std::unique_ptr<Atrc::Mgr::Context> context_;
    
    Atrc::Renderer   *renderer_;
    Atrc::Sampler    *sampler_;
    Atrc::FilmFilter *filmFilter_;

    std::unique_ptr<Atrc::Scene> scene_;

    Atrc::Reporter   *reporter_;
    std::unique_ptr<Atrc::Film> film_;

    Vec2i filmSize_;

public:

    SceneRenderer();

    void StartRendering();

    void EndRendering();
};
