#pragma once

#include <memory>

#include <agz/tracer/core/renderer.h>
#include <agz/factory/factory.h>
#include <agz/tracer/utility/config.h>

AGZ_TRACER_BEGIN

class Camera;
class Film;
class PostProcessor;
class RendererInteractor;
class Renderer;

class RenderSession
{
public:

    struct RenderSetting
    {
        int width  = 1;
        int height = 1;

        real eps = real(3e-4);

        std::shared_ptr<Camera>                     camera;
        std::shared_ptr<FilmFilter>                 film_filter;
        std::shared_ptr<Renderer>                   renderer;
        std::shared_ptr<RendererInteractor>           reporter;
        std::vector<std::shared_ptr<PostProcessor>> post_processors;
    };

    RenderSession() = default;

    RenderSession(std::shared_ptr<Scene> scene, std::unique_ptr<RenderSetting> render_setting) noexcept;

    void execute();

    std::shared_ptr<Scene> scene;
    std::unique_ptr<RenderSetting> render_settings;
};

RenderSession create_render_session(
    std::shared_ptr<Scene> scene,
    const ConfigGroup &rendering_setting_config,
    factory::CreatingContext &context);

std::vector<RenderSession> parse_render_sessions(
    const ConfigGroup &scene_config,
    const ConfigNode &rendering_setting_config,
    factory::CreatingContext &context);

AGZ_TRACER_END
