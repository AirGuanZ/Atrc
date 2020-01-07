#include <cxxopts.hpp>

#include <agz/cli/cli.h>
#include <agz/utility/file.h>

std::optional<Params> parse_opts(int argc, char *argv[])
{
    cxxopts::Options opts("agz-cli", "cli for agz offline renderer");
    opts.add_options("")
        ("s,scene", "scene description", cxxopts::value<std::string>())
        ("d,scene-filename", "scene description filename", cxxopts::value<std::string>())
        ("h,help", "help information");
    auto parse_result = opts.parse(argc, argv);

    if(parse_result.count("help"))
    {
        std::cout << opts.help({ "" }) << std::endl;
        return std::nullopt;
    }

    Params ret;

    const bool has_scene_content  = parse_result.count("scene") != 0;
    const bool has_scene_filename = parse_result.count("scene-filename") != 0;

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
