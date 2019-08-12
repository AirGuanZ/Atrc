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
    -d,--scene-filename SceneDescriptionFilename    | -s,--scene SceneDescription
        若只给了-d，就从其文件中读取场景描述信息
        若只给了-s，就以其内容为场景描述信息，且假装这些信息是从路径“./scene.txt”中读取出的
        若同时给了-s和-d，就以前者为场景描述信息，且假装这些信息是从后者中读取出的
*/
std::optional<Params> parse_opts(int argc, char *argv[]);
