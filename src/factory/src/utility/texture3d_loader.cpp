#include <agz/factory/context.h>
#include <agz/factory/utility/texture3d_loader.h>
#include <agz/utility/image.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

namespace
{
    template<typename Texel>
    texture::texture3d_t<Texel> load_from_binary(std::ifstream &fin)
    {
        int32_t width, height, depth;
        fin.read(reinterpret_cast<char *>(&width), sizeof(width));
        fin.read(reinterpret_cast<char *>(&height), sizeof(height));
        fin.read(reinterpret_cast<char *>(&depth), sizeof(depth));
        if(!fin)
        {
            throw ObjectConstructionException(
                "failed to read width/height/depth from file");
        }

        texture::texture3d_t<Texel> data(depth, height, width);

        for(int32_t z = 0; z < depth; ++z)
        {
            for(int32_t y = 0; y < height; ++y)
            {
                for(int32_t x = 0; x < width; ++x)
                {
                    fin.read(
                        reinterpret_cast<char *>(&data(z, y, x)), sizeof(Texel));
                }
            }
        }

        return data;
    }

    template<typename Texel, typename Func>
    texture::texture3d_t<Texel> load_from_ascii(
        std::ifstream &fin, const Func &read_texel)
    {
        int width, height, depth;
        fin >> width >> height >> depth;
        if(!fin)
        {
            throw ObjectConstructionException(
                "failed to read width/height/depth from file");
        }

        texture::texture3d_t<Texel> data(depth, height, width);
        for(int z = 0; z < depth; ++z)
        {
            for(int y = 0; y < height; ++y)
            {
                for(int x = 0; x < width; ++x)
                {
                    data(z, y, x) = read_texel(fin);
                    if(!fin)
                    {
                        throw ObjectConstructionException(
                            "failed to parse texel data from file");
                    }
                }
            }
        }

        return data;
    }
}

namespace texture3d_load
{

texture::texture3d_t<real> load_real_from_ascii(
    std::ifstream &fin)
{
    return load_from_ascii<real>(fin, [](auto &f)
    {
        real ret;
        f >> ret;
        return ret;
    });
}

texture::texture3d_t<real> load_real_from_binary(
    std::ifstream &fin)
{
    return load_from_binary<real>(fin);
}

texture::texture3d_t<real> load_real_from_images(
    const std::string *filenames, int image_count,
    const factory::PathMapper &path_mapper)
{
    return load_uint8_from_images(
                filenames, image_count, path_mapper)
           .map([](uint8_t texel) { return texel / real(255); });
}

texture::texture3d_t<Spectrum> load_spec_from_ascii(
    std::ifstream &fin)
{
    return load_from_ascii<Spectrum>(fin, [](auto &f)
    {
        Spectrum ret;
        for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
            f >> ret[i];
        return ret;
    });
}

texture::texture3d_t<Spectrum> load_spec_from_binary(
    std::ifstream &fin)
{
    return load_from_binary<Spectrum>(fin);
}

texture::texture3d_t<Spectrum> load_spec_from_images(
    const std::string *filenames, int image_count,
    const factory::PathMapper &path_mapper)
{
    return load_uint24_from_images(
                filenames, image_count, path_mapper)
           .map(math::from_color3b<real>);
}

texture::texture3d_t<uint8_t> load_uint8_from_ascii(std::ifstream &fin)
{
    return load_from_ascii<uint8_t>(fin, [](auto &f)
    {
        uint8_t ret;
        f >> ret;
        return ret;
    });
}

texture::texture3d_t<uint8_t> load_uint8_from_binary(std::ifstream &fin)
{
    return load_from_binary<uint8_t>(fin);
}

texture::texture3d_t<uint8_t> load_uint8_from_images(
    const std::string *filenames, int image_count,
    const factory::PathMapper &path_mapper)
{
    texture::texture3d_t<uint8_t> data;

    int width = -1, height = -1, depth = image_count;
    for(int z = 0; z < depth; ++z)
    {
        const std::string filename = path_mapper.map(filenames[z]);
        const auto slice = img::load_gray_from_file(filename);

        const int new_width  = slice.shape()[1];
        const int new_height = slice.shape()[0];

        if(width < 0)
            width = new_width;
        else if(width != new_width)
        {
            throw ObjectConstructionException(
                "inconsistent image width: " + std::to_string(width) +
                " and " + std::to_string(new_width));
        }

        if(height < 0)
            height = new_height;
        else if(height != new_height)
        {
            throw ObjectConstructionException(
                "inconsistent image height: " + std::to_string(height) +
                " and " + std::to_string(new_height));
        }

        if(!data.is_available())
            data.initialize(depth, height, width);

        for(int y = 0; y < height; ++y)
        {
            for(int x = 0; x < width; ++x)
                data(z, y, x) = slice(y, x);
        }
    }

    return data;
}

texture::texture3d_t<math::color3b> load_uint24_from_ascii(
    std::ifstream &fin)
{
    return load_from_ascii<math::color3b>(fin, [](auto &f)
    {
        math::color3b ret;
        f >> ret.r >> ret.g >> ret.b;
        return ret;
    });
}

texture::texture3d_t<math::color3b> load_uint24_from_binary(
    std::ifstream &fin)
{
    return load_from_binary<math::color3b>(fin);
}

texture::texture3d_t<math::color3b> load_uint24_from_images(
    const std::string *filenames, int image_count,
    const factory::PathMapper &path_mapper)
{
    texture::texture3d_t<math::color3b> data;

    int width = -1, height = -1, depth = image_count;
    for(int z = 0; z < depth; ++z)
    {
        const std::string filename = path_mapper.map(filenames[z]);
        const auto slice = img::load_rgb_from_file(filename);

        const int new_width = slice.shape()[1];
        const int new_height = slice.shape()[0];

        if(width < 0)
            width = new_width;
        else if(width != new_width)
        {
            throw ObjectConstructionException(
                "inconsistent image width: " + std::to_string(width) +
                " and " + std::to_string(new_width));
        }

        if(height < 0)
            height = new_height;
        else if(height != new_height)
        {
            throw ObjectConstructionException(
                "inconsistent image height: " + std::to_string(height) +
                " and " + std::to_string(new_height));
        }

        if(!data.is_available())
            data.initialize(depth, height, width);

        for(int y = 0; y < height; ++y)
        {
            for(int x = 0; x < width; ++x)
            {
                data(z, y, x) = slice(y, x);
            }
        }
    }

    return data;
}

void save_real_to_binary(
    const std::string &filename, const Vec3i &size, const float *data)
{
    std::ofstream fout(filename, std::ios::trunc | std::ios::binary);
    if(!fout)
        throw std::runtime_error("failed to open file: " + filename);

    int32_t w = size.x, h = size.y, d = size.z;
    fout.write(reinterpret_cast<const char *>(&w), sizeof(w));
    fout.write(reinterpret_cast<const char *>(&h), sizeof(h));
    fout.write(reinterpret_cast<const char *>(&d), sizeof(d));

    fout.write(
        reinterpret_cast<const char *>(data), size.product() * sizeof(float));
}

void save_spec_to_binary(
    const std::string &filename, const Vec3i &size, const float *data)
{
    std::ofstream fout(filename, std::ios::trunc | std::ios::binary);
    if(!fout)
        throw std::runtime_error("failed to open file: " + filename);

    int32_t w = size.x, h = size.y, d = size.z;
    fout.write(reinterpret_cast<const char *>(&w), sizeof(w));
    fout.write(reinterpret_cast<const char *>(&h), sizeof(h));
    fout.write(reinterpret_cast<const char *>(&d), sizeof(d));

    fout.write(
        reinterpret_cast<const char *>(data), size.product() * sizeof(float) * 3);
}

void save_uint8_to_binary(
    const std::string &filename, const Vec3i &size, const uint8_t *data)
{
    std::ofstream fout(filename, std::ios::trunc | std::ios::binary);
    if(!fout)
        throw std::runtime_error("failed to open file: " + filename);

    int32_t w = size.x, h = size.y, d = size.z;
    fout.write(reinterpret_cast<const char *>(&w), sizeof(w));
    fout.write(reinterpret_cast<const char *>(&h), sizeof(h));
    fout.write(reinterpret_cast<const char *>(&d), sizeof(d));

    fout.write(
        reinterpret_cast<const char *>(data), size.product() * sizeof(uint8_t));
}

void save_uint24_to_binary(
    const std::string &filename, const Vec3i &size, const math::color3b *data)
{
    std::ofstream fout(filename, std::ios::trunc | std::ios::binary);
    if(!fout)
        throw std::runtime_error("failed to open file: " + filename);

    int32_t w = size.x, h = size.y, d = size.z;
    fout.write(reinterpret_cast<const char *>(&w), sizeof(w));
    fout.write(reinterpret_cast<const char *>(&h), sizeof(h));
    fout.write(reinterpret_cast<const char *>(&d), sizeof(d));

    fout.write(
        reinterpret_cast<const char *>(data), size.product() * sizeof(math::color3b));
}

} // namespace texture3d_load

AGZ_TRACER_END
