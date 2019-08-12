#include <cxxopts.hpp>

#include <agz/cli/cli.h>

#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/film_filter.h>
#include <agz/tracer/core/film.h>
#include <agz/tracer/core/fresnel.h>
#include <agz/tracer/core/geometry.h>
#include <agz/tracer/core/guided_pt_integrator.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/path_tracing_integrator.h>
#include <agz/tracer/core/post_processor.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/reporter.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/core/texture.h>

#include <agz/utility/file.h>

template<typename T>
void print_object_params(const std::string &name, const agz::tracer::obj::ObjectFactory<T> &factory)
{
    //IMPROVE: 现在的输出实在是太TM丑了
    const std::string SPLITTER = std::string(60, '-') + "\n";
    std::cout << "Object Category: " << name << std::endl;
    std::cout << SPLITTER;
    for(auto &p : factory)
    {
        std::cout << p.second->description() << std::endl;
        std::cout << SPLITTER;
    }
}

std::optional<Params> parse_opts(int argc, char *argv[])
{
    cxxopts::Options opts("agz-cli", "cli for agz offline renderer");
    opts.add_options("")
        ("s,scene", "scene description", cxxopts::value<std::string>())
        ("d,scene-filename", "scene description filename", cxxopts::value<std::string>())
        ("h,help", "help information")
        ("l,list", "list all object params");
    auto parse_result = opts.parse(argc, argv);

    if(parse_result.count("help"))
    {
        std::cout << opts.help({ "" }) << std::endl;
        return std::nullopt;
    }
    
    if(parse_result.count("list"))
    {
        using namespace agz::tracer;
        print_object_params("Camera",                      CameraFactory);                      std::cout << std::endl;
        print_object_params("Entity",                      EntityFactory);                      std::cout << std::endl;
        print_object_params("FilmFilter",                  FilmFilterFactory);                  std::cout << std::endl;
        print_object_params("Film",                        FilmFactory);                        std::cout << std::endl;
        print_object_params("Fresnel",                     FresnelFactory);                     std::cout << std::endl;
        print_object_params("Geometry",                    GeometryFactory);                    std::cout << std::endl;
        print_object_params("GuidedPathTracingIntegrator", GuidedPathTracingIntegratorFactory); std::cout << std::endl;
        print_object_params("Light",                       LightFactory);                       std::cout << std::endl;
        print_object_params("Material",                    MaterialFactory);                    std::cout << std::endl;
        print_object_params("PathTracingIntegrator",       PathTracingIntegratorFactory);       std::cout << std::endl;
        print_object_params("PostProcessor",               PostProcessorFactory);               std::cout << std::endl;
        print_object_params("Renderer",                    RendererFactory);                    std::cout << std::endl;
        print_object_params("ProgressReporter",            ProgressReporterFactory);            std::cout << std::endl;
        print_object_params("Sampler",                     SamplerFactory);                     std::cout << std::endl;
        print_object_params("Scene",                       SceneFactory);                       std::cout << std::endl;
        print_object_params("Texture",                     TextureFactory);
        return std::nullopt;
    }

    Params ret;

    bool has_scene_content = parse_result.count("scene") != 0;
    bool has_scene_filename = parse_result.count("scene-filename") != 0;

    if(has_scene_content)
    {
        ret.scene_description = parse_result["scene"].as<std::string>();
        if(has_scene_filename)
            ret.scene_filename = parse_result["scene-filename"].as<std::string>();
        else
            ret.scene_filename = "./scene.txt";
    }
    else if(has_scene_filename)
    {
        ret.scene_filename = parse_result["scene-filename"].as<std::string>();
        ret.scene_description = agz::file::read_txt_file(ret.scene_filename);
    }
    else
        throw ParamParsingException("scene description is unspecified");

    return ret;
}
