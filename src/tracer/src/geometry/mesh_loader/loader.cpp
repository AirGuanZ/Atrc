#include <agz/tracer/utility/logger.h>
#include <agz/utility/misc.h>
#include <agz/utility/string.h>

#include <stl_reader.h>
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "./loader.h"

AGZ_TRACER_BEGIN

namespace mesh
{

static std::vector<Triangle> load_obj(const std::string &filename)
{
    AGZ_HIERARCHY_TRY

    tinyobj::ObjReader reader;
    if(!reader.ParseFromFile(filename))
        throw ObjectConstructionException(reader.Error());

    auto &attrib = reader.GetAttrib();

    auto get_pos = [&](size_t index)
    {
        if(3 * index + 2 >= attrib.vertices.size())
            throw ObjectConstructionException("invalid obj vertex index: out of range");
        return Vec3(attrib.vertices[3 * index + 0],
            attrib.vertices[3 * index + 1],
            attrib.vertices[3 * index + 2]);
    };

    auto get_nor = [&](size_t index)
    {
        if(3 * index + 2 >= attrib.normals.size())
            throw ObjectConstructionException("invalid obj normal index: out of range");
        return Vec3(attrib.normals[3 * index + 0],
            attrib.normals[3 * index + 1],
            attrib.normals[3 * index + 2]);
    };

    auto get_uv = [&](size_t index)
    {
        if(2 * index + 1 >= attrib.texcoords.size())
            throw ObjectConstructionException("invalid obj texcoord index: out of range");
        return Vec2(attrib.texcoords[2 * index + 0],
            attrib.texcoords[2 * index + 1]);
    };

    std::vector<Triangle> build_triangles;
    bool has_invalid_normal = false;

    for(auto &shape : reader.GetShapes())
    {
        for(auto face_vertex_count : shape.mesh.num_face_vertices)
        {
            if(face_vertex_count != 3)
                throw ObjectConstructionException("invalid obj face vertex count: " + std::to_string(+face_vertex_count));
        }

        if(shape.mesh.indices.size() % 3 != 0)
            throw ObjectConstructionException("invalid obj index count: " + std::to_string(shape.mesh.indices.size()));

        build_triangles.reserve(build_triangles.size() + shape.mesh.indices.size() / 3);
        size_t triangle_count = shape.mesh.indices.size() / 3;

        for(size_t i = 0, j = 0; i < triangle_count; ++i, j += 3)
        {
            Triangle tri;
            auto &vtx = tri.vtx;

            for(size_t k = 0; k < 3; ++k)
                vtx[k].pos = get_pos(shape.mesh.indices[j + k].vertex_index);

            for(size_t k = 0; k < 3; ++k)
            {
                auto &idx = shape.mesh.indices[j + k];

                if(idx.normal_index < 0)
                    vtx[k].nor = cross(vtx[1].pos - vtx[0].pos, vtx[2].pos - vtx[0].pos).normalize();
                else
                {
                    vtx[k].nor = get_nor(idx.normal_index);
                    if(!vtx[k].nor)
                    {
                        has_invalid_normal = true;
                        vtx[k].nor = cross(vtx[1].pos - vtx[0].pos, vtx[2].pos - vtx[0].pos).normalize();
                    }
                }

                if(idx.texcoord_index < 0)
                    vtx[k].uv = Vec2();
                else
                    vtx[k].uv = get_uv(idx.texcoord_index);
            }

            build_triangles.push_back(tri);
        }
    }

    if(has_invalid_normal)
        AGZ_LOG1("invalid normal value found");

    AGZ_LOG2("wavefront obj loaded from ", filename);
    AGZ_LOG2("triangle count: ", build_triangles.size());

    return build_triangles;

    AGZ_HIERARCHY_WRAP("in loading wavefront obj from " + filename)
}

static std::vector<Triangle> load_stl(const std::string &filename)
{
    AGZ_HIERARCHY_TRY

    std::vector<Triangle> ret;

    stl_reader::StlMesh<real, int> mesh(filename);
    for(size_t i = 0; i < mesh.num_tris(); ++i)
    {
        Triangle tri;
        auto &vtx = tri.vtx;
        for(int k = 0; k < 3; ++k)
        {
            auto c = mesh.vrt_coords(mesh.tri_corner_ind(i, k));
            vtx[k].pos[0] = c[0];
            vtx[k].pos[1] = c[1];
            vtx[k].pos[2] = c[2];
        }
        for(int k = 0; k < 3; ++k)
            vtx[k].nor = cross(vtx[1].pos - vtx[0].pos, vtx[2].pos - vtx[0].pos).normalize();

        ret.push_back(tri);
    }

    AGZ_LOG2("stl mesh loaded from ", filename);
    AGZ_LOG2("triangle count: ", ret.size());

    return ret;

    AGZ_HIERARCHY_WRAP("in loading stl mesh from " + filename)
}

std::vector<Triangle> load_from_file(const std::string &filename)
{
    if(stdstr::ends_with(filename, ".obj"))
        return load_obj(filename);
    if(stdstr::ends_with(filename, ".stl"))
        return load_stl(filename);
    throw std::runtime_error("unsupported mesh file: " + filename);
}

} // namespace mesh

AGZ_TRACER_END

