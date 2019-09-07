#pragma once

#include "./opengl.h"

inline std::string to_json(const vec3 &v)
{
    return "[" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + "]";
}

inline std::string to_json(float v)
{
    return "[" + std::to_string(v) + "]";
}
