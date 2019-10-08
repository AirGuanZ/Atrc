#pragma once

#include <memory>

#include <agz/tracer/core/renderer.h>
#include <agz/tracer/utility/config.h>

AGZ_TRACER_BEGIN

class Camera;
class Film;
class PostProcessor;
class ProgressReporter;
class Renderer;

class RenderSession
{
public:

    struct RenderSetting
    {
        std::shared_ptr<Camera>                     camera;
        std::shared_ptr<Film>                       film;
        std::shared_ptr<Renderer>                   renderer;
        std::shared_ptr<ProgressReporter>           reporter;
        std::vector<std::shared_ptr<PostProcessor>> post_processors;
    };

    RenderSession(std::shared_ptr<Scene> scene, std::unique_ptr<RenderSetting> &&render_setting) noexcept;

    void execute();

private:

    std::shared_ptr<Scene> scene_;
    std::unique_ptr<RenderSetting> render_settings_;
};

std::vector<RenderSession> parse_render_sessions(
    const ConfigGroup &scene_config,
    const ConfigNode &rendering_setting_config,
    factory::CreatingContext &context);

AGZ_TRACER_END
