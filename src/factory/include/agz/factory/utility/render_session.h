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

        RC<Camera>                     camera;
        RC<FilmFilter>                 film_filter;
        RC<Renderer>                   renderer;
        RC<RendererInteractor>           reporter;
        std::vector<RC<PostProcessor>> post_processors;
    };

    RenderSession() = default;

    RenderSession(RC<Scene> scene, Box<RenderSetting> render_setting) noexcept;

    void execute();

    RC<Scene> scene;
    Box<RenderSetting> render_settings;
};

RenderSession create_render_session(
    RC<Scene> scene,
    const ConfigGroup &rendering_setting_config,
    factory::CreatingContext &context);

std::vector<RenderSession> parse_render_sessions(
    const ConfigGroup &scene_config,
    const ConfigNode &rendering_setting_config,
    factory::CreatingContext &context);

AGZ_TRACER_END
