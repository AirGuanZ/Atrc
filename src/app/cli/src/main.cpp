#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <agz/cli/cli.h>
#include <agz/cli/config_cvt.h>

#include <agz/tracer/core/camera.h>
#include <agz/tracer/factory/factory.h>
#include <agz/tracer/utility/logger.h>
#include <agz/tracer/utility/render_session.h>

#include <agz/utility/misc.h>
#include <agz/utility/string.h>

#ifdef USE_EMBREE
#   include <agz/tracer/utility/embree.h>
#endif

#define WORKING_DIR_PATH_NAME "${working-directory}"
#define SCENE_DESC_PATH_NAME  "${scene-directory}"

class PathMapper : public agz::tracer::factory::PathMapper
{
    std::map<std::string, std::string> replacers_;

public:

    void add_replacer(const std::string &key, const std::string &value)
    {
        replacers_[key] = value;
    }

    std::string map(const std::string &s) const override
    {
        std::string ret(s);
        for(auto &p : replacers_)
            agz::stdstr::replace_(ret, p.first, p.second);
        return absolute(std::filesystem::path(ret)).lexically_normal().string();
    }
};

void run(int argc, char *argv[])
{
    auto params = parse_opts(argc, argv);
    if(!params)
        return;

#ifdef USE_EMBREE
        AGZ_LOG0("initializing embree device");
        agz::tracer::init_embree_device();
        AGZ_SCOPE_GUARD({
			AGZ_LOG0("destroying embree device");
            agz::tracer::destroy_embree_device();
        });
#endif

    PathMapper path_mapper;
    {
        auto working_dir = absolute(std::filesystem::current_path()).lexically_normal().string();
        path_mapper.add_replacer(WORKING_DIR_PATH_NAME, working_dir);
        AGZ_LOG0("working directory: ", working_dir);

        auto scene_dir = absolute(std::filesystem::path(params->scene_filename)).parent_path().lexically_normal().string();
        path_mapper.add_replacer(SCENE_DESC_PATH_NAME, scene_dir);
        AGZ_LOG0("scene directory: ", scene_dir);
    }

    auto root_params       = json_to_config(string_to_json(params->scene_description));
    auto &scene_config     = root_params.child_group("scene");
    auto &rendering_config = root_params.child("rendering");

    agz::tracer::factory::CreatingContext context;
    context.path_mapper = &path_mapper;
    context.reference_root = &scene_config;

    auto scene = context.create<agz::tracer::Scene>(scene_config);

    if(rendering_config.is_array())
    {
        auto &rendering_config_arr = rendering_config.as_array();
        AGZ_LOG0("there is ", rendering_config_arr.size(), " render sessions");
        for(size_t i = 0; i < rendering_config_arr.size(); ++i)
        {
            AGZ_LOG0("processing rendering session [", i, "]");
            auto render_session = create_render_session(scene, rendering_config_arr.at_group(i), context);
            render_session.execute();
        }
    }
    else
    {
        auto render_session = create_render_session(scene, rendering_config.as_group(), context);
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
        set_global_logger(std::make_unique<agz::tracer::StdOutLogger>());

        AGZ_LOG0(">>> AGZ Renderer by AirGuanZ <<<");

#ifdef USE_EMBREE
        AGZ_LOG0("USE_EMBREE = ON");
#else
        AGZ_LOG0("USE_EMBREE = OFF");
#endif

#ifdef USE_OIDN
        AGZ_LOG0("USE_OIDN = ON");
#else
        AGZ_LOG0("USE_OIDN = OFF");
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
