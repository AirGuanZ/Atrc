#pragma once

#include <agz/tracer/common.h>
#include <agz-utils/mesh.h>

AGZ_TRACER_FACTORY_BEGIN

std::vector<mesh::triangle_t> load_bin_mesh(const std::string &filename);

void save_bin_mesh(
    const std::string &filename,
    const void *triangles, size_t triangle_count);

AGZ_TRACER_FACTORY_END
