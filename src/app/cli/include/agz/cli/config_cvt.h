#pragma once

#include <json.hpp>

#include <agz/tracer/utility/config.h>

using JSON = nlohmann::json;

JSON string_to_json(const std::string &str);
JSON config_to_json(const agz::tracer::Config &config);

std::string         json_to_string(const JSON &json);
agz::tracer::Config json_to_config(const JSON &json);
