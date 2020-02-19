#include <agz/tracer/factory/creator/texture2d_creators.h>
#include <agz/tracer/factory/raw/texture2d.h>
#include <agz/utility/image.h>

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

        std::shared_ptr<Texture2D> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = init_common_params(params);
            const int grid_count = params.child_int("grid_count");
            const Spectrum color1 = params.child_spectrum("color1");
            const Spectrum color2 = params.child_spectrum("color2");
            return create_checker_board(common_params, grid_count, color1, color2);
        }
    };

    class ConstantCreator : public Creator<Texture2D>
    {
    public:

        std::string name() const override
        {
            return "constant";
        }

        std::shared_ptr<Texture2D> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = init_common_params(params);
            const auto texel = params.child_spectrum("texel");
            return create_constant2d_texture(common_params, texel);
        }
    };

    class GradientCreator : public Creator<Texture2D>
    {
    public:

        std::string name() const override
        {
            return "gradient";
        }

        std::shared_ptr<Texture2D> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = init_common_params(params);
            const Spectrum color1 = params.child_spectrum("color1");
            const Spectrum color2 = params.child_spectrum("color2");
            return create_gradient_texture(common_params, color1, color2);
        }
    };

    class HDRCreator : public Creator<Texture2D>
    {
        mutable std::map<std::string, std::shared_ptr<const Image2D<math::color3f>>> filename2data_;

    public:

        std::string name() const override
        {
            return "hdr";
        }

        std::shared_ptr<Texture2D> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = init_common_params(params);
            const auto filename = context.path_mapper->map(params.child_str("filename"));
            const auto sample = params.child_str_or("sample", "linear");

            std::shared_ptr<const Image2D<math::color3f>> data;
            if(auto it = filename2data_.find(filename); it != filename2data_.end())
                data = it->second;
            else
            {
                const auto raw_data = img::load_rgb_from_hdr_file(filename);
                if(!raw_data.is_available())
                    throw ObjectConstructionException("failed to load texture from " + filename);

                data = std::make_shared<Image2D<math::color3f>>(std::move(raw_data));
                filename2data_[filename] = data;
            }

            return create_hdr_texture(common_params, std::move(data), sample);
        }
    };

    class ImageCreator : public Creator<Texture2D>
    {
        mutable std::map<std::string, std::shared_ptr<const Image2D<math::color3b>>> filename2data_;

    public:

        std::string name() const override
        {
            return "image";
        }

        std::shared_ptr<Texture2D> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = init_common_params(params);
            const auto filename = context.path_mapper->map(params.child_str("filename"));
            const auto sample = params.child_str_or("sample", "linear");

            std::shared_ptr<const Image2D<math::color3b>> data;
            if(auto it = filename2data_.find(filename); it != filename2data_.end())
                data = it->second;
            else
            {
                const auto raw_data = img::load_rgb_from_file(filename);
                if(!raw_data.is_available())
                    throw ObjectConstructionException("failed to load texture from " + filename);

                data = std::make_shared<Image2D<math::color3b>>(std::move(raw_data));
                filename2data_[filename] = data;
            }

            return create_image_texture(common_params, std::move(data), sample);
        }
    };

    class TextureScalerCreator : public Creator<Texture2D>
    {
    public:

        std::string name() const override
        {
            return "scale";
        }

        std::shared_ptr<Texture2D> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = init_common_params(params);
            const auto scale = params.child_spectrum("scale");
            const auto internal = context.create<Texture2D>(params.child_group("internal"));
            return create_texture2d_scaler(common_params, scale, std::move(internal));
        }
    };

    class TextureAdderCreator : public Creator<Texture2D>
    {
    public:

        std::string name() const override
        {
            return "add";
        }

        std::shared_ptr<Texture2D> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = init_common_params(params);
            const auto lhs = context.create<Texture2D>(params.child_group("lhs"));
            const auto rhs = context.create<Texture2D>(params.child_group("rhs"));
            return create_texture2d_adder(common_params, std::move(lhs), std::move(rhs));
        }
    };

    class TextureMultiplierCreator : public Creator<Texture2D>
    {
    public:

        std::string name() const override
        {
            return "mul";
        }

        std::shared_ptr<Texture2D> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = init_common_params(params);
            const auto lhs = context.create<Texture2D>(params.child_group("lhs"));
            const auto rhs = context.create<Texture2D>(params.child_group("rhs"));
            return create_texture2d_multiplier(common_params, std::move(lhs), std::move(rhs));
        }
    };

    class TextureLuminanceClassifierCreator : public Creator<Texture2D>
    {
    public:

        std::string name() const override
        {
            return "lum_classify";
        }

        std::shared_ptr<Texture2D> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = init_common_params(params);
            const auto internal = context.create<Texture2D>(params.child_group("internal"));
            const auto threshold = context.create<Texture2D>(params.child_group("threshold"));
            const auto higher = context.create<Texture2D>(params.child_group("high"));
            const auto lower = context.create<Texture2D>(params.child_group("low"));
            return create_texture2d_luminance_classifier(
                common_params,
                std::move(internal),
                std::move(threshold),
                std::move(higher),
                std::move(lower));
        }
    };

    class TextureReverseCreator : public Creator<Texture2D>
    {
    public:

        std::string name() const override
        {
            return "reverse";
        }

        std::shared_ptr<Texture2D> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = init_common_params(params);
            const auto internal = context.create<Texture2D>(params.child_group("internal"));
            return create_texture2d_reverse(common_params, std::move(internal));
        }
    };

} // namespace texture

void initialize_texture2d_factory(Factory<Texture2D> &factory)
{
    factory.add_creator(std::make_unique<texture::CheckerBoardCreator>());
    factory.add_creator(std::make_unique<texture::ConstantCreator>());
    factory.add_creator(std::make_unique<texture::GradientCreator>());
    factory.add_creator(std::make_unique<texture::HDRCreator>());
    factory.add_creator(std::make_unique<texture::ImageCreator>());
    factory.add_creator(std::make_unique<texture::TextureScalerCreator>());
    factory.add_creator(std::make_unique<texture::TextureAdderCreator>());
    factory.add_creator(std::make_unique<texture::TextureMultiplierCreator>());
    factory.add_creator(std::make_unique<texture::TextureLuminanceClassifierCreator>());
    factory.add_creator(std::make_unique<texture::TextureReverseCreator>());
}

AGZ_TRACER_FACTORY_END
