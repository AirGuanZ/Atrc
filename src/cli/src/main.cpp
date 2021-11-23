#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <agz/cli/cli.h>
#include <agz/factory/factory.h>
#include <agz/tracer/tracer.h>

#include <agz-utils/misc.h>
#include <agz-utils/string.h>

void run(int argc, char *argv[])
{
    auto params = parse_opts(argc, argv);
    if(!params)
        return;

#ifdef USE_EMBREE
        AGZ_INFO("initializing embree device");
        agz::tracer::init_embree_device();
        AGZ_SCOPE_EXIT{
            AGZ_INFO("destroying embree device");
            agz::tracer::destroy_embree_device();
        };
#endif

    agz::tracer::factory::BasicPathMapper path_mapper;
    {
        const auto working_dir = absolute(
            std::filesystem::current_path()).lexically_normal().string();
        path_mapper.add_replacer(AGZ_FACTORY_WORKING_DIR_PATH_NAME, working_dir);
        AGZ_INFO("working directory: {}", working_dir);

        const auto scene_dir = absolute(
            std::filesystem::path(params->scene_filename))
                .parent_path().lexically_normal().string();
        path_mapper.add_replacer(AGZ_FACTORY_SCENE_DESC_PATH_NAME, scene_dir);
        AGZ_INFO("scene directory: {}", scene_dir);
    }

    const auto root_params = agz::tracer::factory::json_to_config(
        agz::tracer::factory::string_to_json(params->scene_description));
    const auto &scene_config = root_params.child_group("scene");
    const auto &rendering_config = root_params.child("rendering");

    agz::tracer::factory::CreatingContext context;
    context.path_mapper = &path_mapper;
    context.reference_root = &scene_config;

    auto scene = context.create<agz::tracer::Scene>(scene_config);

    if(rendering_config.is_array())
    {
        const auto &rendering_config_arr = rendering_config.as_array();
        AGZ_INFO("there is {} render sessions", rendering_config_arr.size());
        for(size_t i = 0; i < rendering_config_arr.size(); ++i)
        {
            AGZ_INFO("processing rendering session [{}]", i);
            auto render_session = create_render_session(
                scene, rendering_config_arr.at_group(i), context);
            render_session.execute();
        }
    }
    else
    {
        auto render_session = create_render_session(
            scene, rendering_config.as_group(), context);
        render_session.execute();
    }
}

#if defined(_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif

int main(int argc, char *argv[])
{
#if defined(_WIN32) && defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    try
    {
        AGZ_INFO(">>> Atrc Renderer by AirGuanZ <<<");

#ifdef USE_EMBREE
        AGZ_INFO("USE_EMBREE = ON");
#else
        AGZ_INFO("USE_EMBREE = OFF");
#endif

#ifdef USE_OIDN
        AGZ_INFO("USE_OIDN = ON");
#else
        AGZ_INFO("USE_OIDN = OFF");
#endif

        run(argc, argv);

        return 0;
    }
    catch(const std::exception &e)
    {
        std::vector<std::string> msgs;
        agz::misc::extract_hierarchy_exceptions(e, std::back_inserter(msgs));
        for(auto &m : msgs)
            std::cout << m << std::endl;
    }
    catch(...)
    {
        std::cout << "an unknown error occurred" << std::endl;
    }

    return -1;
}
