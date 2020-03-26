#include <fstream>

#include <agz/factory/utility/bin_mesh.h>

AGZ_TRACER_FACTORY_BEGIN

std::vector<mesh::triangle_t> load_bin_mesh(const std::string &filename)
{
    std::ifstream fin(filename, std::ios::binary | std::ios::in);
    if(!fin)
        throw std::runtime_error("failed to open file: " + filename);

    size_t triangle_count;
    fin.read(reinterpret_cast<char*>(&triangle_count), sizeof(triangle_count));
    if(!fin)
        throw std::runtime_error(
            "failed to load triangle count from " + filename);

    const size_t byte_size = sizeof(mesh::triangle_t) * triangle_count;
    std::vector<mesh::triangle_t> ret(triangle_count);
    fin.read(reinterpret_cast<char*>(ret.data()), byte_size);
    if(!fin)
        throw std::runtime_error(
            "failed to load triangle data from " + filename);

    return ret;
}

void save_bin_mesh(
    const std::string &filename,
    const void *triangles, size_t triangle_count)
{
    std::ofstream fout(filename, std::ios::binary | std::ios::trunc);
    if(!fout)
        throw std::runtime_error("failed to open file: " + filename);

    fout.write(reinterpret_cast<char*>(&triangle_count), sizeof(triangle_count));
    if(!fout)
        throw std::runtime_error(
            "failed to write triangle count to " + filename);

    const size_t byte_size = sizeof(mesh::triangle_t) * triangle_count;
    fout.write(reinterpret_cast<const char*>(triangles), byte_size);
    if(!fout)
        throw std::runtime_error(
            "failed to write triangle data to " + filename);
}

AGZ_TRACER_FACTORY_END
