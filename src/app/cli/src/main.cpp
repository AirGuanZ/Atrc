#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <agz/cli/cli.h>
#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/post_processor.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/reporter.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/factory/factory.h>
#include <agz/tracer/factory/creator/scene_creators.h>
#include <agz/tracer/utility/logger.h>
#include <agz/tracer/utility/config_cvt.h>
#include <agz/utility/image.h>
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

struct RenderingSettings
{
    std::shared_ptr<agz::tracer::Camera>                     camera;
    std::shared_ptr<agz::tracer::Film>                       film;
    std::shared_ptr<agz::tracer::Renderer>                   renderer;
    std::shared_ptr<agz::tracer::ProgressReporter>           reporter;
    std::vector<std::shared_ptr<agz::tracer::PostProcessor>> post_processors;
};

RenderingSettings parse_rendering_settings(agz::tracer::Config &rendering_config, agz::tracer::factory::CreatingContext &context)
{
    using namespace agz::tracer;
    using namespace agz;

    auto &film_params = rendering_config.child_group("film");
    int film_width = film_params.child_int("width");
    int film_height = film_params.child_int("height");

    auto film_width_over_height = static_cast<real>(film_width) / film_height;

    auto film_aspect_child = std::make_shared<ConfigValue>(std::to_string(film_width_over_height));
    rendering_config.child_group("camera").insert_child("aspect", film_aspect_child);

    RenderingSettings settings;

    AGZ_LOG1("creating camera");
    auto &camera_params = rendering_config.child_group("camera");
    settings.camera = context.create<Camera>(camera_params);

    AGZ_LOG1("creating renderer");
    auto &renderer_params = rendering_config.child_group("renderer");
    settings.renderer = context.create<Renderer>(renderer_params);

    AGZ_LOG1("creating progress reporter");
    auto &reporter_params = rendering_config.child_group("reporter");
    settings.reporter = context.create<ProgressReporter>(reporter_params);

    AGZ_LOG1("creating render target");
    settings.film = context.create<Film>(film_params);
    AGZ_LOG1("resolution: (", film_width, ", ", film_height, ")");

    if(auto node = rendering_config.find_child("post_processors"))
    {
        AGZ_LOG1("creating post processors");
        auto &arr = node->as_array();
        settings.post_processors.reserve(arr.size());
        for(size_t i = 0; i != arr.size(); ++i)
        {
            auto &group = arr.at(i).as_group();
            if(stdstr::ends_with(group.child_str("type"), "//"))
                continue;
            auto elem = context.create<PostProcessor>(group);
            settings.post_processors.push_back(elem);
        }
    }
    else
        AGZ_LOG1("no post processor");

    return settings;
}

void render(agz::tracer::Scene &scene, agz::tracer::Config &rendering_settings, agz::tracer::factory::CreatingContext &context)
{
    auto settings = parse_rendering_settings(rendering_settings, context);

    AGZ_LOG0("start rendering");
    scene.set_camera(settings.camera);
    scene.start_rendering();
    settings.renderer->render(scene, *settings.reporter, settings.film.get());

    auto img     = settings.film->image();
    auto gbuffer = settings.film->gbuffer();

    AGZ_LOG0("running post processers");
    for(auto p : settings.post_processors)
        p->process(img, gbuffer);
}

void run(int argc, char *argv[])
{
    using namespace agz::img;
    using namespace agz::stdstr;
    using namespace agz::tracer;
    namespace fs = std::filesystem;

    auto params = parse_opts(argc, argv);
    if(!params)
        return;

#ifdef USE_EMBREE
        AGZ_LOG0("initializing embree device");
        init_embree_device();
        AGZ_SCOPE_GUARD({
            destroy_embree_device();
        });
#endif

    PathMapper path_mapper;
    {
        auto working_dir = absolute(fs::current_path()).lexically_normal().string();
        path_mapper.add_replacer(WORKING_DIR_PATH_NAME, working_dir);
        AGZ_LOG0("working directory: ", working_dir);

        auto scene_dir = absolute(fs::path(params->scene_filename)).parent_path().lexically_normal().string();
        path_mapper.add_replacer(SCENE_DESC_PATH_NAME, scene_dir);
        AGZ_LOG0("scene directory: ", scene_dir);
    }

    auto scene_config_parent = json_to_config(string_to_json(params->scene_description));
    auto &scene_config       = scene_config_parent.child_group("scene");
    auto &rendering_config   = scene_config_parent.child("rendering");

    factory::CreatingContext context;
    context.path_mapper = &path_mapper;
    context.reference_root = &scene_config;

    AGZ_LOG0("creating scene");
    auto scene = factory::build_scene(scene_config, context);

    if(rendering_config.is_group())
    {
        render(*scene, rendering_config.as_group(), context);
    }
    else if(rendering_config.is_array())
    {
        auto &arr = rendering_config.as_array();
        for(size_t i = 0; i < arr.size(); ++i)
        {
            AGZ_LOG0("processing rendering config [", i, "]");
            render(*scene, arr.at_group(i), context);
        }
    }
    else
        throw ObjectConstructionException("invalid rendering_config type");

    AGZ_LOG0("destroying embree device");
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
