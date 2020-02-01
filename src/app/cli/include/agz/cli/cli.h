#pragma once

#include <optional>
#include <stdexcept>
#include <string>

class ParamParsingException : public std::invalid_argument
{
public:

    using invalid_argument::invalid_argument;
};

struct Params
{
    std::string scene_description;
    std::string scene_filename;
};

/*
    -d,--scene-filename SceneDescriptionFilename | -s,--scene SceneDescription

        -d only: load scene desc from SceneDescriptionFilename
        -s only: use SceneDescription as scene desc and assume that it's loaded from './scene.txt'
        -d and -s: use SceneDescription as scene desc and assume that it's loaded from SceneDescriptionFilename
*/
std::optional<Params> parse_opts(int argc, char *argv[]);
