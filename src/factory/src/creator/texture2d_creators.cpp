#include <agz/factory/creator/texture2d_creators.h>
#include <agz/tracer/create/texture2d.h>
#include <agz/tracer/create/texture3d.h>
#include <agz-utils/image.h>

AGZ_TRACER_FACTORY_BEGIN

namespace texture
{

    Texture2DCommonParams init_common_params(const ConfigGroup &params)
    {
        Texture2DCommonParams ret;
        
        ret.inv_u = params.child_int_or("inv_u", 0) != 0;
        ret.inv_v = params.child_int_or("inv_v", 0) != 0;
        ret.swap_uv = params.child_int_or("swap_uv", 0) != 0;
        
        if(params.find_child("transform"))
            ret.transform = params.child_transform2("transform");
        
        ret.wrap_u = params.child_str_or("wrap_u", "clamp");
        ret.wrap_v = params.child_str_or("wrap_v", "clamp");
        ret.inv_gamma = params.child_real_or("inv_gamma", 1);
        
        return ret;
    }
    
    class CheckerBoardCreator : public Creator<Texture2D>
    {
    public:

        std::string name() const override
        {
            return "checker_board";
        }

        RC<Texture2D> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = init_common_params(params);
            const int grid_count = params.child_int("grid_count");
            const Spectrum color1 = params.child_spectrum("color1");
            const Spectrum color2 = params.child_spectrum("color2");
            return create_checker_board(
                common_params, grid_count, color1, color2);
        }
    };

    class ConstantCreator : public Creator<Texture2D>
    {
    public:

        std::string name() const override
        {
            return "constant";
        }

        RC<Texture2D> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = init_common_params(params);
            const auto texel = params.child_spectrum("texel");
            return create_constant2d_texture(common_params, texel);
        }
    };

    class HDRCreator : public Creator<Texture2D>
    {
        mutable std::map<std::string, RC<const Image2D<math::color3f>>>
            filename2data_;

    public:

        std::string name() const override
        {
            return "hdr";
        }

        RC<Texture2D> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = init_common_params(params);
            const auto filename =
                context.path_mapper->map(params.child_str("filename"));
            const auto sample =
                params.child_str_or("sample", "linear");

            RC<const Image2D<math::color3f>> data;
            if(auto it = filename2data_.find(filename);
               it != filename2data_.end())
                data = it->second;
            else
            {
                const auto raw_data = img::load_rgb_from_hdr_file(filename);
                if(!raw_data.is_available())
                    throw ObjectConstructionException(
                        "failed to load texture from " + filename);

                data = newRC<Image2D<math::color3f>>(std::move(raw_data));
                filename2data_[filename] = data;
            }

            return create_hdr_texture(common_params, std::move(data), sample);
        }
    };

    class ImageCreator : public Creator<Texture2D>
    {
        mutable std::map<std::string, RC<const Image2D<math::color3b>>>
            filename2data_;

    public:

        std::string name() const override
        {
            return "image";
        }

        RC<Texture2D> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = init_common_params(params);
            const auto filename =
                context.path_mapper->map(params.child_str("filename"));
            const auto sample =
                params.child_str_or("sample", "linear");

            RC<const Image2D<math::color3b>> data;
            if(auto it = filename2data_.find(filename);
               it != filename2data_.end())
                data = it->second;
            else
            {
                const auto raw_data = img::load_rgb_from_file(filename);
                if(!raw_data.is_available())
                    throw ObjectConstructionException(
                        "failed to load texture from " + filename);

                data = newRC<Image2D<math::color3b>>(std::move(raw_data));
                filename2data_[filename] = data;
            }

            return create_image_texture(common_params, std::move(data), sample);
        }
    };

    class SolidImageCreator : public Creator<Texture2D>
    {
    public:

        std::string name() const override
        {
            return "solid_image";
        }

        RC<Texture2D> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            auto tex3d = context.create<Texture3D>(params.child_group("tex3d"));
            return create_solid_image_texture(std::move(tex3d));
        }
    };

} // namespace texture

void initialize_texture2d_factory(Factory<Texture2D> &factory)
{
    factory.add_creator(newBox<texture::CheckerBoardCreator>());
    factory.add_creator(newBox<texture::ConstantCreator>());
    factory.add_creator(newBox<texture::HDRCreator>());
    factory.add_creator(newBox<texture::ImageCreator>());
    factory.add_creator(newBox<texture::SolidImageCreator>());
}

AGZ_TRACER_FACTORY_END
