#include <agz/tracer/factory/factory.h>
#include <agz/tracer/utility/texture3d_loader.h>
#include <agz/utility/image.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

texture::texture3d_t<real> texture3d_load::load_gray_from_ascii(std::ifstream &fin)
{
    int width, height, depth;
    fin >> width >> height >> depth;
    if(!fin)
        throw ObjectConstructionException("failed to read width/height/depth from file");

    texture::texture3d_t<real> data(depth, height, width);
    for(int z = 0; z < depth; ++z)
    {
        for(int y = 0; y < height; ++y)
        {
            for(int x = 0; x < width; ++x)
            {
                fin >> data(z, y, x);
                if(!fin)
                    throw ObjectConstructionException("failed to parse texel data from file");
            }
        }
    }

    return data;
}

texture::texture3d_t<real> texture3d_load::load_gray_from_binary(std::ifstream &fin)
{
    int32_t width, height, depth;
    fin.read(reinterpret_cast<char *>(&width), sizeof(width));
    fin.read(reinterpret_cast<char *>(&height), sizeof(height));
    fin.read(reinterpret_cast<char *>(&depth), sizeof(depth));
    if(!fin)
        throw ObjectConstructionException("failed to read width/height/depth from file");

    texture::texture3d_t<real> data(depth, height, width);

    for(int32_t z = 0; z < depth; ++z)
    {
        for(int32_t y = 0; y < height; ++y)
        {
            for(int32_t x = 0; x < width; ++x)
            {
                float texel;
                fin.read(reinterpret_cast<char *>(&texel), sizeof(texel));
                data(z, y, x) = misc::to_local_endian<misc::endian_type::little>(texel);
            }
        }
    }

    return data;
}

texture::texture3d_t<real> texture3d_load::load_gray_from_images(
    const std::string *filenames, int image_count, const factory::PathMapper &path_mapper)
{
    texture::texture3d_t<real> data;

    int width = -1, height = -1, depth = image_count;
    for(int z = 0; z < depth; ++z)
    {
        const std::string filename = path_mapper.map(filenames[z]);
        const auto slice = img::load_gray_from_file(filename);

        const int new_width = slice.shape()[1];
        const int new_height = slice.shape()[0];

        if(width < 0)
            width = new_width;
        else if(width != new_width)
        {
            throw ObjectConstructionException(
                "inconsistent image width: " + std::to_string(width) + " and " + std::to_string(new_width));
        }

        if(height < 0)
            height = new_height;
        else if(height != new_height)
        {
            throw ObjectConstructionException(
                "inconsistent image height: " + std::to_string(height) + " and " + std::to_string(new_height));
        }

        if(!data.is_available())
            data.initialize(depth, height, width);

        for(int y = 0; y < height; ++y)
        {
            for(int x = 0; x < width; ++x)
            {
                data(z, y, x) = slice(y, x) / 255.0f;
            }
        }
    }

    return data;
}

texture::texture3d_t<Spectrum> texture3d_load::load_rgb_from_ascii(std::ifstream &fin)
{
    int width, height, depth;
    fin >> width >> height >> depth;
    if(!fin)
        throw ObjectConstructionException("failed to read width/height/depth from file");

    texture::texture3d_t<Spectrum> data(depth, height, width);
    for(int z = 0; z < depth; ++z)
    {
        for(int y = 0; y < height; ++y)
        {
            for(int x = 0; x < width; ++x)
            {
                Spectrum spec;
                for(int c = 0; c < SPECTRUM_COMPONENT_COUNT; ++c)
                    fin >> spec[c];
                if(!fin)
                    throw ObjectConstructionException("failed to parse texel data from file");
                data(z, y, x) = spec;
            }
        }
    }

    return data;
}

texture::texture3d_t<Spectrum> texture3d_load::load_rgb_from_binary(std::ifstream &fin)
{
    int32_t width, height, depth;
    fin.read(reinterpret_cast<char *>(&width), sizeof(width));
    fin.read(reinterpret_cast<char *>(&height), sizeof(height));
    fin.read(reinterpret_cast<char *>(&depth), sizeof(depth));
    if(!fin)
        throw ObjectConstructionException("failed to read width/height/depth from file");

    texture::texture3d_t<Spectrum> data(depth, height, width);

    for(int32_t z = 0; z < depth; ++z)
    {
        for(int32_t y = 0; y < height; ++y)
        {
            for(int32_t x = 0; x < width; ++x)
            {
                Spectrum texel;
                for(int c = 0; c < SPECTRUM_COMPONENT_COUNT; ++c)
                {
                    real component;
                    fin.read(reinterpret_cast<char *>(&component), sizeof(component));
                    texel[c] = misc::to_local_endian<misc::endian_type::little>(component);
                }
                data(z, y, x) = texel;
            }
        }
    }

    return data;
}

texture::texture3d_t<Spectrum> texture3d_load::load_rgb_from_images(
    const std::string *filenames, int image_count, const factory::PathMapper &path_mapper)
{
    texture::texture3d_t<Spectrum> data;

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
                "inconsistent image width: " + std::to_string(width) + " and " + std::to_string(new_width));
        }

        if(height < 0)
            height = new_height;
        else if(height != new_height)
        {
            throw ObjectConstructionException(
                "inconsistent image height: " + std::to_string(height) + " and " + std::to_string(new_height));
        }

        if(!data.is_available())
            data.initialize(depth, height, width);

        for(int y = 0; y < height; ++y)
        {
            for(int x = 0; x < width; ++x)
            {
                data(z, y, x) = math::from_color3b<real>(slice(y, x));
            }
        }
    }

    return data;
}

AGZ_TRACER_END
