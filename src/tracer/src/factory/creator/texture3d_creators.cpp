#include <fstream>

#include <agz/tracer/factory/creator/texture3d_creators.h>
#include <agz/tracer/factory/raw/texture3d.h>
#include <agz/tracer/utility/texture3d_loader.h>

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

        std::shared_ptr<Texture3D> create(const ConfigGroup &params, CreatingContext &context) const override
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

        static texture::texture3d_t<real> read_from_images(const ConfigArray &filename_array, const PathMapper &path_mapper)
        {
            if(!filename_array.size())
                throw ObjectConstructionException("empty image filename array");

            std::vector<std::string> filenames(filename_array.size());
            for(size_t i = 0; i < filename_array.size(); ++i)
                filenames[i] = filename_array.at_str(i);
            return texture3d_load::load_gray_from_images(filenames.data(), int(filenames.size()), path_mapper);
        }

    public:

        std::string name() const override
        {
            return "gray_grid";
        }

        std::shared_ptr<Texture3D> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            texture::texture3d_t<real> data;
            if(auto *str_child = params.find_child_value("ascii_filename"))
            {
                std::string filename = context.path_mapper->map(str_child->as_str());
                data = read_from_ascii(filename);
            }
            else if(str_child = params.find_child_value("binary_filename"); str_child)
            {
                std::string filename = context.path_mapper->map(str_child->as_str());
                data = read_from_binary(filename);
            }
            else if(auto arr_child = params.find_child_array("image_filenames"))
                data = read_from_images(*arr_child, *context.path_mapper);
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

        static texture::texture3d_t<Spectrum> read_from_images(const ConfigArray &filename_array, const PathMapper &path_mapper)
        {
            if(!filename_array.size())
                throw ObjectConstructionException("empty image filename array");

            std::vector<std::string> filenames(filename_array.size());
            for(size_t i = 0; i < filename_array.size(); ++i)
                filenames[i] = filename_array.at_str(i);
            return texture3d_load::load_rgb_from_images(filenames.data(), int(filenames.size()), path_mapper);
        }

    public:

        std::string name() const override
        {
            return "spectrum_grid";
        }

        std::shared_ptr<Texture3D> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            texture::texture3d_t<Spectrum> data;
            if(auto *str_child = params.find_child_value("ascii_filename"))
                data = read_from_ascii(context.path_mapper->map(str_child->as_str()));
            else if(str_child = params.find_child_value("binary_filename"); str_child)
                data = read_from_binary(context.path_mapper->map(str_child->as_str()));
            else if(auto arr_child = params.find_child_array("image_filenames"))
                data = read_from_images(*arr_child, *context.path_mapper);
            else
                throw ObjectConstructionException("texel data filename not provided");

            return create_spectrum_grid_point3d(parse_common_params(params), std::move(data));
        }
    };

    class Texture3DAdderCreator : public Creator<Texture3D>
    {
    public:

        std::string name() const override
        {
            return "add";
        }

        std::shared_ptr<Texture3D> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = parse_common_params(params);
            const auto lhs = context.create<Texture3D>(params.child_group("lhs"));
            const auto rhs = context.create<Texture3D>(params.child_group("rhs"));
            return create_texture3d_adder(common_params, std::move(lhs), std::move(rhs));
        }
    };

    class Texture3DMultiplierCreator : public Creator<Texture3D>
    {
    public:

        std::string name() const override
        {
            return "mul";
        }

        std::shared_ptr<Texture3D> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = parse_common_params(params);
            const auto lhs = context.create<Texture3D>(params.child_group("lhs"));
            const auto rhs = context.create<Texture3D>(params.child_group("rhs"));
            return create_texture3d_multiplier(common_params, std::move(lhs), std::move(rhs));
        }
    };

    class Texture3DScaleCreator : public Creator<Texture3D>
    {
    public:

        std::string name() const override
        {
            return "scale";
        }

        std::shared_ptr<Texture3D> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = parse_common_params(params);
            const auto internal = context.create<Texture3D>(params.child_group("internal"));
            const auto scale    = params.child_spectrum("scale");
            return create_texture3d_scaler(common_params, std::move(internal), scale);
        }
    };

    class Texture3DLumClassifierCreator : public Creator<Texture3D>
    {
    public:

        std::string name() const override
        {
            return "lum_classify";
        }

        std::shared_ptr<Texture3D> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = parse_common_params(params);
            const auto lhs           = context.create<Texture3D>(params.child_group("lhs"));
            const auto rhs           = context.create<Texture3D>(params.child_group("rhs"));
            const auto less_or_equal = context.create<Texture3D>(params.child_group("less_or_equal"));
            const auto greater       = context.create<Texture3D>(params.child_group("greater"));
            return create_texture3d_lum_classifier(
                common_params, std::move(lhs), std::move(rhs), std::move(less_or_equal), std::move(greater));
        }
    };
}

void initialize_texture3d_factory(Factory<Texture3D> &factory)
{
    factory.add_creator(std::make_unique<Constant3DCreator>());
    factory.add_creator(std::make_unique<GrayGridPoint3DCreator>());
    factory.add_creator(std::make_unique<SpectrumGridPoint3DCreator>());
    factory.add_creator(std::make_unique<Texture3DAdderCreator>());
    factory.add_creator(std::make_unique<Texture3DMultiplierCreator>());
    factory.add_creator(std::make_unique<Texture3DScaleCreator>());
    factory.add_creator(std::make_unique<Texture3DLumClassifierCreator>());
}

AGZ_TRACER_FACTORY_END
