#include <agz/tracer/core/post_processor.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/factory/factory.h>
#include <agz/tracer/utility/logger.h>
#include <agz/tracer/utility/render_session.h>

#include <agz/utility/string.h>

AGZ_TRACER_BEGIN

namespace
{
    std::unique_ptr<RenderSession::RenderSetting> parse_rendering_settings(Config rendering_config, factory::CreatingContext &context)
    {
        auto settings = std::make_unique<RenderSession::RenderSetting>();

        auto &film_params = rendering_config.child_group("film");
        int film_width = film_params.child_int("width");
        int film_height = film_params.child_int("height");

        auto film_width_over_height = static_cast<real>(film_width) / film_height;

        auto film_aspect_child = std::make_shared<ConfigValue>(std::to_string(film_width_over_height));
        rendering_config.child_group("camera").insert_child("aspect", film_aspect_child);

        AGZ_LOG1("creating camera");
        auto &camera_params = rendering_config.child_group("camera");
        settings->camera = context.create<Camera>(camera_params);

        AGZ_LOG1("creating renderer");
        auto &renderer_params = rendering_config.child_group("renderer");
        settings->renderer = context.create<Renderer>(renderer_params);

        AGZ_LOG1("creating progress reporter");
        auto &reporter_params = rendering_config.child_group("reporter");
        settings->reporter = context.create<ProgressReporter>(reporter_params);

        AGZ_LOG1("creating render target");
        settings->film = context.create<Film>(film_params);
        AGZ_LOG1("resolution: (", film_width, ", ", film_height, ")");

        if(auto node = rendering_config.find_child("post_processors"))
        {
            AGZ_LOG1("creating post processors");
            auto &arr = node->as_array();
            settings->post_processors.reserve(arr.size());
            for(size_t i = 0; i != arr.size(); ++i)
            {
                auto &group = arr.at(i).as_group();
                if(stdstr::ends_with(group.child_str("type"), "//"))
                    continue;
                auto elem = context.create<PostProcessor>(group);
                settings->post_processors.push_back(elem);
            }
        }
        else
            AGZ_LOG1("no post processor");

        return settings;
    }
}

RenderSession::RenderSession(std::shared_ptr<Scene> scene, std::unique_ptr<RenderSetting> render_setting) noexcept
    : scene_(std::move(scene)), render_settings_(std::move(render_setting))
{
    
}

void RenderSession::execute()
{
    AGZ_LOG0("start rendering");

    scene_->set_camera(render_settings_->camera);
    render_settings_->renderer->render(*scene_, *render_settings_->reporter, render_settings_->film.get());

    auto img = render_settings_->film->image();
    auto gbuffer = render_settings_->film->gbuffer();

    AGZ_LOG0("running post processors");

    for(auto &p : render_settings_->post_processors)
        p->process(img, gbuffer);
}

RenderSession create_render_session(
    std::shared_ptr<Scene> scene,
    const ConfigGroup &rendering_setting_config,
    factory::CreatingContext &context)
{
    auto setting = parse_rendering_settings(rendering_setting_config, context);
    return RenderSession(std::move(scene), std::move(setting));
}

std::vector<RenderSession> parse_render_sessions(
    const ConfigGroup &scene_config,
    const ConfigNode &rendering_setting_config,
    factory::CreatingContext &context)
{
    std::vector<RenderSession> ret;
    auto scene = context.create<Scene>(scene_config);
    
    if(rendering_setting_config.is_group())
    {
        auto setting = parse_rendering_settings(rendering_setting_config.as_group(), context);
        ret.emplace_back(std::move(scene), std::move(setting));
        return ret;
    }

    auto &arr = rendering_setting_config.as_array();
    for(size_t i = 0; i < arr.size(); ++i)
    {
        auto setting = parse_rendering_settings(arr.at_group(i), context);
        ret.emplace_back(scene, std::move(setting));
    }
    return ret;
}

AGZ_TRACER_END
