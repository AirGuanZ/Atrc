#include <fstream>

#include <agz/tracer/factory/creator/texture3d_creators.h>
#include <agz/tracer/factory/raw/texture3d.h>
#include <agz/utility/image.h>
#include <agz/utility/misc.h>

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
            auto common_params = parse_common_params(params);
            Spectrum texel = params.child_spectrum("texel");
            return create_constant3d_texture(common_params, texel);
        }
    };

    class GrayGridPoint3DCreator : public Creator<Texture3D>
    {
        /**
         * @brief 从文本文件中读取体数据
         *
         * 格式：
         * width : int
         * height: int
         * depth : int
         * for z in 0 to depth
         *     for y in 0 to height
         *         for x in 0 to width
         *             texel: float
         */
        static texture::texture3d_t<real> read_from_ascii(const std::string &filename)
        {
            std::ifstream fin(filename, std::ios::in);
            if(!fin)
                throw ObjectConstructionException("failed to open file: " + filename);

            int width, height, depth;
            fin >> width >> height >> depth;
            if(!fin)
                throw ObjectConstructionException("failed to read width/height/depth from " + filename);

            texture::texture3d_t<real> data(depth, height, width);
            for(int z = 0; z < depth; ++z)
            {
                for(int y = 0; y < height; ++y)
                {
                    for(int x = 0; x < width; ++x)
                    {
                        fin >> data(z, y, x);
                        if(!fin)
                            throw ObjectConstructionException("failed to parse texel data from " + filename);
                    }
                }
            }

            return data;
        }

        /**
         * @brief 从二进制文件中读取体数据
         *
         * 存储使用小端序
         *
         * 格式：
         * width : int32_t
         * height: int32_t
         * depth : int32_t
         * for z in 0 to depth
         *     for y in 0 to height
         *         for x in 0 to width
         *             texel: float
         */
        static texture::texture3d_t<real> read_from_binary(const std::string &filename)
        {
            std::ifstream fin(filename, std::ios::in | std::ios::binary);
            if(!fin)
                throw ObjectConstructionException("failed to open file: " + filename);

            int32_t width, height, depth;
            fin.read(reinterpret_cast<char*>(&width),  sizeof(width));
            fin.read(reinterpret_cast<char*>(&height), sizeof(height));
            fin.read(reinterpret_cast<char*>(&depth),  sizeof(depth));
            if(!fin)
                throw ObjectConstructionException("failed to read width/height/depth from file: " + filename);

            texture::texture3d_t<real> data(depth, height, width);

            for(int32_t z = 0; z < depth; ++z)
            {
                for(int32_t y = 0; y < height; ++y)
                {
                    for(int32_t x = 0; x < width; ++x)
                    {
                        float texel;
                        fin.read(reinterpret_cast<char*>(&texel), sizeof(texel));
                        data(z, y, x) = misc::to_local_endian<misc::endian_type::little>(texel);
                    }
                }
            }

            return data;
        }

        static texture::texture3d_t<real> read_from_images(const ConfigArray &filename_array, CreatingContext &context)
        {
            if(!filename_array.size())
                throw ObjectConstructionException("empty image filename array");

            texture::texture3d_t<real> data;

            int width = -1, height = -1, depth = static_cast<int>(filename_array.size());
            for(int z = 0; z < depth; ++z)
            {
                std::string filename = context.path_mapper->map(filename_array.at_str(z));
                auto slice = img::load_gray_from_file(filename);

                int new_width  = slice.shape()[1];
                int new_height = slice.shape()[0];

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
                data = read_from_images(*arr_child, context);
            else
                throw ObjectConstructionException("texel data filename not provided");

            return create_gray_grid_point3d(parse_common_params(params), std::move(data));
        }
    };

    class SpectrumGridPoint3DCreator : public Creator<Texture3D>
    {
        /**
         * @brief 从文本文件中读取体数据
         *
         * 格式：
         * width : int
         * height: int
         * depth : int
         * for z in 0 to depth
         *     for y in 0 to height
         *         for x in 0 to width
         *             texel: float * SPECTRUM_COMPONENT_COUNT
         */
        static texture::texture3d_t<Spectrum> read_from_ascii(const std::string &filename)
        {
            std::ifstream fin(filename, std::ios::in);
            if(!fin)
                throw ObjectConstructionException("failed to open file: " + filename);

            int width, height, depth;
            fin >> width >> height >> depth;
            if(!fin)
                throw ObjectConstructionException("failed to read width/height/depth from " + filename);

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
                            throw ObjectConstructionException("failed to parse texel data from " + filename);
                        data(z, y, x) = spec;
                    }
                }
            }

            return data;
        }

        /**
         * @brief 从二进制文件中读取体数据
         *
         * 存储使用小端序
         *
         * 格式：
         * width : int32_t
         * height: int32_t
         * depth : int32_t
         * for z in 0 to depth
         *     for y in 0 to height
         *         for x in 0 to width
         *             texel: float * SPECTRUM_COMPONENT_COUNT
         */
        static texture::texture3d_t<Spectrum> read_from_binary(const std::string &filename)
        {
            std::ifstream fin(filename, std::ios::in | std::ios::binary);
            if(!fin)
                throw ObjectConstructionException("failed to open file: " + filename);

            int32_t width, height, depth;
            fin.read(reinterpret_cast<char*>(&width),  sizeof(width));
            fin.read(reinterpret_cast<char*>(&height), sizeof(height));
            fin.read(reinterpret_cast<char*>(&depth),  sizeof(depth));
            if(!fin)
                throw ObjectConstructionException("failed to read width/height/depth from file: " + filename);

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
                            fin.read(reinterpret_cast<char*>(&component), sizeof(component));
                            texel[c] = misc::to_local_endian<misc::endian_type::little>(component);
                        }
                        data(z, y, x) = texel;
                    }
                }
            }

            return data;
        }

        static texture::texture3d_t<Spectrum> read_from_images(const ConfigArray &filename_array, CreatingContext &context)
        {
            if(!filename_array.size())
                throw ObjectConstructionException("empty image filename array");

            texture::texture3d_t<Spectrum> data;

            int width = -1, height = -1, depth = static_cast<int>(filename_array.size());
            for(int z = 0; z < depth; ++z)
            {
                std::string filename = context.path_mapper->map(filename_array.at_str(z));
                auto slice = img::load_rgb_from_file(filename);

                int new_width  = slice.shape()[1];
                int new_height = slice.shape()[0];

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
                data = read_from_images(*arr_child, context);
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
            auto common_params = parse_common_params(params);
            auto lhs = context.create<Texture3D>(params.child_group("lhs"));
            auto rhs = context.create<Texture3D>(params.child_group("rhs"));
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
            auto common_params = parse_common_params(params);
            auto lhs = context.create<Texture3D>(params.child_group("lhs"));
            auto rhs = context.create<Texture3D>(params.child_group("rhs"));
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
            auto common_params = parse_common_params(params);
            auto internal = context.create<Texture3D>(params.child_group("internal"));
            auto scale    = params.child_spectrum("scale");
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
            auto common_params = parse_common_params(params);
            auto lhs           = context.create<Texture3D>(params.child_group("lhs"));
            auto rhs           = context.create<Texture3D>(params.child_group("rhs"));
            auto less_or_equal = context.create<Texture3D>(params.child_group("less_or_equal"));
            auto greater       = context.create<Texture3D>(params.child_group("greater"));
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
