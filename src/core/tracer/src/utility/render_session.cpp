#include <agz/tracer/core/post_processor.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/factory/factory.h>
#include <agz/tracer/factory/raw/film_filter.h>
#include <agz/tracer/utility/logger.h>
#include <agz/tracer/utility/render_session.h>

#include <agz/utility/string.h>

AGZ_TRACER_BEGIN

namespace
{
    std::unique_ptr<RenderSession::RenderSetting> parse_rendering_settings(Config rendering_config, factory::CreatingContext &context)
    {
        auto settings = std::make_unique<RenderSession::RenderSetting>();

        const int film_width = rendering_config.child_int("width");
        const int film_height = rendering_config.child_int("height");

        const auto film_width_over_height = static_cast<real>(film_width) / film_height;

        const auto film_width_child = std::make_shared<ConfigValue>(std::to_string(film_width));
        const auto film_height_child = std::make_shared<ConfigValue>(std::to_string(film_height));
        const auto film_aspect_child = std::make_shared<ConfigValue>(std::to_string(film_width_over_height));
        rendering_config.child_group("camera").insert_child("film_width",  film_width_child);
        rendering_config.child_group("camera").insert_child("film_height", film_height_child);
        rendering_config.child_group("camera").insert_child("film_aspect", film_aspect_child);

        AGZ_INFO("creating render target");
        settings->width  = film_width;
        settings->height = film_height;
        if(auto group = rendering_config.find_child_group("film_filter"))
            settings->film_filter = context.create<FilmFilter>(*group);
        else
            settings->film_filter = create_box_filter(real(0.5));
        AGZ_INFO("resolution: ({}, {})", film_width, film_height);

        AGZ_INFO("creating camera");
        const auto &camera_params = rendering_config.child_group("camera");
        settings->camera = context.create<Camera>(camera_params, film_width, film_height);

        AGZ_INFO("creating renderer");
        const auto &renderer_params = rendering_config.child_group("renderer");
        settings->renderer = context.create<Renderer>(renderer_params);

        AGZ_INFO("creating progress reporter");
        const auto &reporter_params = rendering_config.child_group("reporter");
        settings->reporter = context.create<ProgressReporter>(reporter_params);

        if(auto node = rendering_config.find_child("post_processors"))
        {
            AGZ_INFO("creating post processors");
            const auto &arr = node->as_array();

            settings->post_processors.reserve(arr.size());
            for(size_t i = 0; i != arr.size(); ++i)
            {
                const auto &group = arr.at(i).as_group();
                if(stdstr::ends_with(group.child_str("type"), "//"))
                    continue;
                const auto elem = context.create<PostProcessor>(group);
                settings->post_processors.push_back(elem);
            }
        }
        else
            AGZ_INFO("no post processor");

        return settings;
    }
}

RenderSession::RenderSession(std::shared_ptr<Scene> scene, std::unique_ptr<RenderSetting> render_setting) noexcept
    : scene(std::move(scene)), render_settings(std::move(render_setting))
{
    
}

void RenderSession::execute()
{
    AGZ_INFO("start rendering");

    scene->set_camera(render_settings->camera);
    scene->start_rendering();

    FilmFilterApplier filter_applier(render_settings->width, render_settings->height, render_settings->film_filter);

    RenderTarget render_target = render_settings->renderer->render(filter_applier, *scene, *render_settings->reporter);

    AGZ_INFO("running post processors");

    for(auto &p : render_settings->post_processors)
        p->process(render_target);
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
    const auto scene = context.create<Scene>(scene_config);
    
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
