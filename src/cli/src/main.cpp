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
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/utility/logger.h>
#include <agz/tracer/utility/scene_builder.h>
#include <agz/tracer_utility/config_cvt.h>
#include <agz/utility/image.h>
#include <agz/utility/misc.h>

#ifdef USE_EMBREE
#   include <agz/tracer/utility/embree.h>
#endif // #ifdef USE_EMBREE

struct RenderingSettings
{
    agz::tracer::Camera                     *camera   = nullptr;
    agz::tracer::Film                       *film     = nullptr;
    agz::tracer::Renderer                   *renderer = nullptr;
    agz::tracer::ProgressReporter           *reporter = nullptr;
    std::vector<agz::tracer::PostProcessor*> post_processors;
};

RenderingSettings parse_rendering_settings(agz::tracer::Config &rendering_config, agz::tracer::obj::ObjectInitContext &obj_init_ctx)
{
    using namespace agz::tracer;

    using namespace agz::tracer;

    auto &film_params = rendering_config.child_group("film");
    int film_width = film_params.child_int("width");
    int film_height = film_params.child_int("height");

    auto film_width_over_height = static_cast<real>(film_width) / film_height;

    auto film_aspect_child = std::make_shared<ConfigValue>(std::to_string(film_width_over_height));
    rendering_config.child_group("camera").insert_child("aspect", film_aspect_child);

    RenderingSettings settings;

    AGZ_LOG1("creating camera");
    auto &camera_params = rendering_config.child_group("camera");
    settings.camera = CameraFactory.create(camera_params, obj_init_ctx);

    AGZ_LOG1("creating renderer");
    auto &renderer_params = rendering_config.child_group("renderer");
    settings.renderer = RendererFactory.create(renderer_params, obj_init_ctx);

    AGZ_LOG1("creating progress reporter");
    auto &reporter_params = rendering_config.child_group("reporter");
    settings.reporter = ProgressReporterFactory.create(reporter_params, obj_init_ctx);

    AGZ_LOG1("creating render target");
    settings.film = FilmFactory.create(film_params, obj_init_ctx);
    AGZ_LOG1("resolution: (", film_width, ", ", film_height, ")");

    if(auto node = rendering_config.find_child("post_processors"))
    {
        AGZ_LOG1("creating post processors");
        auto &arr = node->as_array();
        settings.post_processors.reserve(arr.size());
        for(size_t i = 0; i != arr.size(); ++i)
        {
            auto &group = arr.at(i).as_group();
            if(agz::stdstr::ends_with(group.child_str("type"), "//"))
                continue;
            auto elem = PostProcessorFactory.create(group, obj_init_ctx);
            settings.post_processors.push_back(elem);
        }
    }
    else
        AGZ_LOG1("no post processor");

    return settings;
}

void render(agz::tracer::Scene &scene, agz::tracer::Config &rendering_settings, const agz::tracer::obj::ObjectInitContext &obj_init_ctx)
{
    agz::tracer::Arena arena;
    auto init_ctx = obj_init_ctx;
    init_ctx.arena = &arena;

    auto settings = parse_rendering_settings(rendering_settings, init_ctx);

    AGZ_LOG0("start rendering");
    scene.set_camera(settings.camera);
    scene.start_rendering();
    settings.renderer->render(
        scene, *settings.reporter, settings.film);

    settings.film->end();
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
        agz::tracer::init_embree_device();
        AGZ_SCOPE_GUARD({
            AGZ_LOG0("destroying embree device");
            agz::tracer::destroy_embree_device();
        });
#endif

    PathManager path_mgr;
    {
        auto working_dir = absolute(fs::current_path()).lexically_normal().string();
        path_mgr.add_replacer(WORKING_DIR_PATH_NAME, working_dir);
        AGZ_LOG0("working directory: ", working_dir);

        auto scene_dir = absolute(fs::path(params->scene_filename)).parent_path().lexically_normal().string();
        path_mgr.add_replacer(SCENE_DESC_PATH_NAME, scene_dir);
        AGZ_LOG0("scene directory: ", scene_dir);
    }

    auto scene_config_parent = json_to_config(string_to_json(params->scene_description));
    auto &scene_config       = scene_config_parent.child_group("scene");
    auto &rendering_config   = scene_config_parent.child("rendering");

    Arena arena;
    obj::ObjectInitContext obj_init_ctx;
    obj_init_ctx.path_mgr       = &path_mgr;
    obj_init_ctx.arena          = &arena;
    obj_init_ctx.reference_root = &scene_config;

    AGZ_LOG0("creating scene");
    Scene *scene = SceneBuilder::build(scene_config, obj_init_ctx);

    if(rendering_config.is_group())
    {
        render(*scene, rendering_config.as_group(), obj_init_ctx);
        return;
    }

    if(rendering_config.is_array())
    {
        auto &arr = rendering_config.as_array();
        for(size_t i = 0; i < arr.size(); ++i)
        {
            AGZ_LOG0("processing rendering config [", i, "]");
            render(*scene, arr.at_group(i), obj_init_ctx);
        }
        return;
    }

    throw agz::tracer::ObjectConstructionException("invalid rendering_config type");
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

#ifdef USE_BCD
        AGZ_LOG0("USE_BCD = ON");
#else
        AGZ_LOG0("USE_BCD = OFF");
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
