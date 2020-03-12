#include <fstream>

#include <agz/factory/creator/texture3d_creators.h>
#include <agz/factory/utility/texture3d_loader.h>
#include <agz/tracer/create/texture3d.h>

AGZ_TRACER_FACTORY_BEGIN

namespace
{
    Texture3DCommonParams parse_common_params(const ConfigGroup &params)
    {
        Texture3DCommonParams ret;
        ret.from_params(params);
        return ret;
    }

    class Constant3DCreator : public Creator<Texture3D>
    {
    public:

        std::string name() const override
        {
            return "constant";
        }

        RC<Texture3D> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = parse_common_params(params);
            const Spectrum texel = params.child_spectrum("texel");
            return create_constant3d_texture(common_params, texel);
        }
    };

    class GrayGridPoint3DCreator : public Creator<Texture3D>
    {
        static texture::texture3d_t<real> read_from_ascii(const std::string &filename)
        {
            std::ifstream fin(filename, std::ios::in);
            if(!fin)
                throw ObjectConstructionException("failed to open file: " + filename);
            return texture3d_load::load_gray_from_ascii(fin);
        }

        static texture::texture3d_t<real> read_from_binary(const std::string &filename)
        {
            std::ifstream fin(filename, std::ios::in | std::ios::binary);
            if(!fin)
                throw ObjectConstructionException("failed to open file: " + filename);
            return texture3d_load::load_gray_from_binary(fin);
        }

        static texture::texture3d_t<real> read_from_images(
            const ConfigArray &filename_array, const PathMapper &path_mapper)
        {
            if(!filename_array.size())
                throw ObjectConstructionException("empty image filename array");

            std::vector<std::string> filenames(filename_array.size());
            for(size_t i = 0; i < filename_array.size(); ++i)
                filenames[i] = filename_array.at_str(i);
            return texture3d_load::load_gray_from_images(
                filenames.data(), int(filenames.size()), path_mapper);
        }

    public:

        std::string name() const override
        {
            return "gray_grid";
        }

        RC<Texture3D> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            RC<const Image3D<real>> data;
            if(auto *str_child = params.find_child_value("ascii_filename"))
            {
                std::string filename = context.path_mapper->map(str_child->as_str());
                data = newRC<Image3D<real>>(read_from_ascii(filename));
            }
            else if(str_child = params.find_child_value("binary_filename"); str_child)
            {
                std::string filename = context.path_mapper->map(str_child->as_str());
                data = newRC<Image3D<real>>(read_from_binary(filename));
            }
            else if(auto arr_child = params.find_child_array("image_filenames"))
                data = newRC<Image3D<real>>(read_from_images(*arr_child, *context.path_mapper));
            else
                throw ObjectConstructionException("texel data filename not provided");

            return create_gray_grid_point3d(parse_common_params(params), std::move(data));
        }
    };

    class SpectrumGridPoint3DCreator : public Creator<Texture3D>
    {
        static texture::texture3d_t<Spectrum> read_from_ascii(const std::string &filename)
        {
            std::ifstream fin(filename, std::ios::in);
            if(!fin)
                throw ObjectConstructionException("failed to open file: " + filename);
            return texture3d_load::load_rgb_from_ascii(fin);
        }

        static texture::texture3d_t<Spectrum> read_from_binary(const std::string &filename)
        {
            std::ifstream fin(filename, std::ios::in | std::ios::binary);
            if(!fin)
                throw ObjectConstructionException("failed to open file: " + filename);
            return texture3d_load::load_rgb_from_binary(fin);
        }

        static texture::texture3d_t<Spectrum> read_from_images(
            const ConfigArray &filename_array, const PathMapper &path_mapper)
        {
            if(!filename_array.size())
                throw ObjectConstructionException("empty image filename array");

            std::vector<std::string> filenames(filename_array.size());
            for(size_t i = 0; i < filename_array.size(); ++i)
                filenames[i] = filename_array.at_str(i);
            return texture3d_load::load_rgb_from_images(
                filenames.data(), int(filenames.size()), path_mapper);
        }

    public:

        std::string name() const override
        {
            return "spectrum_grid";
        }

        RC<Texture3D> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            RC<const texture::texture3d_t<Spectrum>> data;
            if(auto *str_child = params.find_child_value("ascii_filename"))
            {
                data = newBox<Image3D<Spectrum>>(
                    read_from_ascii(context.path_mapper->map(str_child->as_str())));
            }
            else if(str_child = params.find_child_value("binary_filename"); str_child)
            {
                data = newBox<Image3D<Spectrum>>(
                    read_from_binary(context.path_mapper->map(str_child->as_str())));
            }
            else if(auto arr_child = params.find_child_array("image_filenames"))
            {
                data = newBox<Image3D<Spectrum>>(
                    read_from_images(*arr_child, *context.path_mapper));
            }
            else
                throw ObjectConstructionException("texel data filename not provided");

            return create_spectrum_grid_point3d(parse_common_params(params), std::move(data));
        }
    };

}

void initialize_texture3d_factory(Factory<Texture3D> &factory)
{
    factory.add_creator(newBox<Constant3DCreator>());
    factory.add_creator(newBox<GrayGridPoint3DCreator>());
    factory.add_creator(newBox<SpectrumGridPoint3DCreator>());
}

AGZ_TRACER_FACTORY_END
