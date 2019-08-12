#pragma once

#include <agz/tracer/utility/math.h>

AGZ_TRACER_BEGIN

namespace mesh
{

struct Vertex
{
    Vec3 pos;
    Vec3 nor;
    Vec2 uv;
};

struct Triangle
{
    Vertex vtx[3];
};

/**
 * @brief load triangles from .obj .stl
 */
std::vector<Triangle> load_from_file(const std::string &filename);

} // namespace mesh

AGZ_TRACER_END

