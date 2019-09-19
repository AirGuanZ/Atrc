#include <filesystem>
#include <iostream>
#include <fstream>
#include <set>

#include <agz/utility/math.h>
#include <agz/utility/string.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <agz/obj_to_scene/tiny_obj_loader.h>

namespace fs = std::filesystem;
using namespace tinyobj;

using Vec2 = agz::math::vec2f;
using Vec3 = agz::math::vec3f;

struct Vertex
{
    Vec3 pos;
    Vec3 nor;
    Vec2 uv;
};

// 1. 用tiny obj loader加载模型和材质
// 2. 对每个shape进行后面的步骤
// 3. 对shape中的具有相同material id的faces进行后面的步骤
// 4. 将这些faces输出到新obj中
// 5. 输出entity信息

const std::string USAGE_MSG = R"___(
obj_to_scene obj_filename
)___";

const std::string OUTPUT_JSON_FILENAME = "./obj_to_scene_output/entities.json";
const std::string OUTPUT_ASSERT_DIR    = "./obj_to_scene_output/assets";

std::vector<Vertex> collect_vertices_with_material_id(int material_id, const mesh_t &mesh, const attrib_t &attrib)
{
    std::vector<Vertex> object_vertices;
    int triangle_count = static_cast<int>(mesh.indices.size() / 3);

    auto get_pos = [&](size_t index)
    {
        if(3 * index + 2 >= attrib.vertices.size())
            throw std::runtime_error("invalid obj vertex index: out of range");
        return Vec3(attrib.vertices[3 * index + 0],
            attrib.vertices[3 * index + 1],
            attrib.vertices[3 * index + 2]);
    };

    auto get_nor = [&](size_t index)
    {
        if(3 * index + 2 >= attrib.normals.size())
            throw std::runtime_error("invalid obj normal index: out of range");
        return Vec3(attrib.normals[3 * index + 0],
            attrib.normals[3 * index + 1],
            attrib.normals[3 * index + 2]);
    };

    auto get_uv = [&](size_t index)
    {
        if(2 * index + 1 >= attrib.texcoords.size())
            throw std::runtime_error("invalid obj texcoord index: out of range");
        return Vec2(attrib.texcoords[2 * index + 0],
            attrib.texcoords[2 * index + 1]);
    };

    for(int tri_idx = 0, i = 0; tri_idx < triangle_count; ++tri_idx, i += 3)
    {
        if(mesh.material_ids[tri_idx] != material_id)
            continue;

        Vertex vtx[3];

        for(int j = 0; j < 3; ++j)
            vtx[j].pos = get_pos(mesh.indices[i + j].vertex_index);

        for(int j = 0; j < 3; ++j)
        {
            auto &idx = mesh.indices[i + j];

            if(idx.normal_index < 0)
                vtx[j].nor = cross(vtx[1].pos - vtx[0].pos, vtx[2].pos - vtx[0].pos);
            else
                vtx[j].nor = get_nor(idx.normal_index);
            if(!vtx[j].nor)
                vtx[j].nor = cross(vtx[1].pos - vtx[0].pos, vtx[2].pos - vtx[0].pos);

            if(idx.texcoord_index < 0)
                vtx[j].uv = Vec2();
            else
                vtx[j].uv = get_uv(idx.texcoord_index);
        }

        object_vertices.push_back(vtx[0]);
        object_vertices.push_back(vtx[1]);
        object_vertices.push_back(vtx[2]);
    }

    return object_vertices;
}

std::string material_texture(const Vec3 &scalar, const std::string &filename)
{
    std::stringstream sst;

    std::string scalar_str = "[" + std::to_string(scalar.x) + ", "
                                 + std::to_string(scalar.y) + ", "
                                 + std::to_string(scalar.z) + "]";

    if(filename.empty())
    {
        sst << R"__({ "type": "constant", "texel": )__";
        sst << scalar_str;
        sst << "}";

        return sst.str();
    }

    sst << R"__({ "type": "scale", "scale": )__";
    sst << scalar_str;
    sst << R"__(, "internal": { "type": "image", "inv_v": true, "filename": ")__";
    sst << agz::stdstr::replace("${scene-directory}" + filename, "\\", "/") << "\"}}";

    return sst.str();
}

std::string material_to_string(const material_t &mat)
{
    Vec3 kd(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
    Vec3 ks(mat.specular[0], mat.specular[1], mat.specular[2]);
    float ns = mat.shininess;

    std::stringstream sst;
    sst << R"__({ "type": "mtl", "kd": )__";
    sst << material_texture(kd, mat.diffuse_texname);
    sst << R"__(, "ks": )__";
    sst << material_texture(ks, mat.specular_texname);
    sst << R"__(, "ns": )__";
    sst << material_texture(Vec3(ns), mat.specular_highlight_texname);
    sst << "}";

    return sst.str();
}

std::string object_to_entity(const std::vector<Vertex> &vertices, const std::string &material_str, int object_index)
{
    std::string obj_name = agz::stdstr::align_right(std::to_string(object_index), 10, '0') + ".obj";
    std::string filename = OUTPUT_ASSERT_DIR + "/" + obj_name;
    std::ofstream fout(filename, std::ofstream::trunc);
    if(!fout)
        throw std::runtime_error("failed to open output obj file: " + filename);

    for(auto &vtx : vertices)
    {
        fout << "v "  << vtx.pos.x << " " << vtx.pos.y << " " << vtx.pos.z << std::endl;
        fout << "vn " << vtx.nor.x << " " << vtx.nor.y << " " << vtx.nor.z << std::endl;
        fout << "vt " << vtx.uv.x  << " " << vtx.uv.y  << std::endl;
    }

    for(size_t i = 0; i < vertices.size(); i += 3)
    {
        fout << "f "
             << i+1 << "/" << i+1 << "/" << i+1 << " "
             << i+2 << "/" << i+2 << "/" << i+2 << " "
             << i+3 << "/" << i+3 << "/" << i+3
             << std::endl;
    }

    fout.close();

    std::string geometry = R"___(
    {
      "type": "double_sided",
      "internal": {
        "type": "triangle_bvh",
        "filename": "OBJ_FILENAME",
        "transform": [ ]
      }
    }
    )___";
    agz::stdstr::replace_(geometry, "OBJ_FILENAME", "${scene-directory}/" + filename);

    return "{ \"type\" : \"geometric\", "
            "\"geometry\" : " + geometry + ", \"material\" : " + material_str + "}";
}

int run(int argc, char *argv[])
{
    if(argc < 2)
    {
        std::cout << agz::stdstr::trim(USAGE_MSG) << std::endl;
        return 0;
    }

    std::string obj_filename = argv[1];
    ObjReader reader;
    if(!reader.ParseFromFile(obj_filename))
        throw std::runtime_error("failed to load obj from " + obj_filename);

    if(!fs::exists(OUTPUT_ASSERT_DIR) && !fs::create_directories(OUTPUT_ASSERT_DIR))
        throw std::runtime_error("failed to create output directory: " + OUTPUT_ASSERT_DIR);
    
    auto &materials = reader.GetMaterials();
    auto &shapes    = reader.GetShapes();
    auto &attrib    = reader.GetAttrib();

    int object_count = 0;
    std::string output_string;

    std::vector<std::string> material_descs;
    for(auto &mat : materials)
        material_descs.push_back(material_to_string(mat));
    material_descs.emplace_back(
        "{ \"##\": \"agz_default_material\","
          "\"type\": \"ideal_diffuse\","
          "\"albedo\": { \"type\": \"all_zero\" } }");

    for(auto &shape : shapes)
    {
        auto &mesh = shape.mesh;

        std::cout << "start processing mesh " << shape.name << std::endl;

        for(int n : mesh.num_face_vertices)
        {
            if(n != 3)
                throw std::runtime_error("invalid number of face vertices: " + std::to_string(n));
        }
        
        std::set<int> material_ids;
        for(int id : mesh.material_ids)
        {
            if(id < 0)
                id = static_cast<int>(material_descs.size()) - 1;
            if(static_cast<size_t>(id) >= material_descs.size())
                throw std::runtime_error("invalid material id: " + std::to_string(id));
            material_ids.insert(id);
        }

        std::cout << material_ids.size() << " materials in total" << std::endl;

        for(int id : material_ids)
        {
            auto object_vertices = collect_vertices_with_material_id(id, mesh, attrib);
            auto entity_string   = object_to_entity(object_vertices, material_descs[id], object_count);
            output_string += entity_string + ",\n";

            std::string mat_name = static_cast<size_t>(id) >= material_descs.size() - 1 ? "default" : materials[id].name;
            std::cout << "processed object " << object_count << " with material " << mat_name << std::endl;

            object_count += 1;
        }
    }

    std::ofstream fout(OUTPUT_JSON_FILENAME, std::ofstream::trunc);
    if(!fout)
    {
        throw std::runtime_error("failed to open json output filename: " + OUTPUT_JSON_FILENAME);
    }
    fout << output_string;

    return 0;
}

int main(int argc, char *argv[])
{
    try
    {
        return run(argc, argv);
    }
    catch(const std::exception &err)
    {
        std::cout << err.what() << std::endl;
        return -1;
    }
}
