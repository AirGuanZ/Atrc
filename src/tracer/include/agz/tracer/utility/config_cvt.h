#pragma once

#include <json.hpp>

#include <agz/tracer/utility/config.h>

AGZ_TRACER_BEGIN

using JSON = nlohmann::json;

JSON string_to_json(const std::string &str);
JSON config_to_json(const Config &config);

std::string json_to_string(const JSON &json);
Config      json_to_config(const JSON &json);

AGZ_TRACER_END
