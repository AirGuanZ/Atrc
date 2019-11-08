#include <agz/tracer/factory/creator/texture_creators.h>
#include <agz/tracer/factory/raw/texture.h>

AGZ_TRACER_FACTORY_BEGIN

namespace texture
{

    TextureCommonParams init_common_params(const ConfigGroup &params)
    {
        TextureCommonParams ret;
        
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
    
    class CheckerBoardCreator : public Creator<Texture>
    {
    public:

        std::string name() const override
        {
            return "checker_board";
        }

        std::shared_ptr<Texture> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto common_params = init_common_params(params);
            int grid_count = params.child_int("grid_count");
            Spectrum color1 = params.child_spectrum("color1");
            Spectrum color2 = params.child_spectrum("color2");
            return create_checker_board(common_params, grid_count, color1, color2);
        }
    };

    class ConstantCreator : public Creator<Texture>
    {
    public:

        std::string name() const override
        {
            return "constant";
        }

        std::shared_ptr<Texture> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto common_params = init_common_params(params);
            auto texel = params.child_spectrum("texel");
            return create_constant_texture(common_params, texel);
        }
    };

    class GradientCreator : public Creator<Texture>
    {
    public:

        std::string name() const override
        {
            return "gradient";
        }

        std::shared_ptr<Texture> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto common_params = init_common_params(params);
            Spectrum color1 = params.child_spectrum("color1");
            Spectrum color2 = params.child_spectrum("color2");
            return create_gradient_texture(common_params, color1, color2);
        }
    };

    class HDRCreator : public Creator<Texture>
    {
    public:

        std::string name() const override
        {
            return "hdr";
        }

        std::shared_ptr<Texture> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto common_params = init_common_params(params);
            auto filename = context.path_mapper->map(params.child_str("filename"));
            auto sample = params.child_str_or("sample", "linear");
            return create_hdr_texture(common_params, filename, sample);
        }
    };

    class ImageCreator : public Creator<Texture>
    {
    public:

        std::string name() const override
        {
            return "image";
        }

        std::shared_ptr<Texture> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto common_params = init_common_params(params);
            auto filename = context.path_mapper->map(params.child_str("filename"));
            auto sample = params.child_str_or("sample", "linear");
            return create_image_texture(common_params, filename, sample);
        }
    };

    class TextureScalerCreator : public Creator<Texture>
    {
    public:

        std::string name() const override
        {
            return "scale";
        }

        std::shared_ptr<Texture> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto common_params = init_common_params(params);
            auto scale = params.child_spectrum("scale");
            auto internal = context.create<Texture>(params.child_group("internal"));
            return create_texture_scaler(common_params, scale, std::move(internal));
        }
    };

    class TextureAdderCreator : public Creator<Texture>
    {
    public:

        std::string name() const override
        {
            return "add";
        }

        std::shared_ptr<Texture> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto common_params = init_common_params(params);
            auto lhs = context.create<Texture>(params.child_group("lhs"));
            auto rhs = context.create<Texture>(params.child_group("rhs"));
            return create_texture_adder(common_params, std::move(lhs), std::move(rhs));
        }
    };

    class TextureMultiplierCreator : public Creator<Texture>
    {
    public:

        std::string name() const override
        {
            return "mul";
        }

        std::shared_ptr<Texture> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto common_params = init_common_params(params);
            auto lhs = context.create<Texture>(params.child_group("lhs"));
            auto rhs = context.create<Texture>(params.child_group("rhs"));
            return create_texture_multiplier(common_params, std::move(lhs), std::move(rhs));
        }
    };

    class TextureLuminanceClassifierCreator : public Creator<Texture>
    {
    public:

        std::string name() const override
        {
            return "lum_classify";
        }

        std::shared_ptr<Texture> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto common_params = init_common_params(params);
            auto internal = context.create<Texture>(params.child_group("internal"));
            auto threshold = context.create<Texture>(params.child_group("threshold"));
            auto higher = context.create<Texture>(params.child_group("high"));
            auto lower = context.create<Texture>(params.child_group("low"));
            return create_luminance_classifier(
                common_params,
                std::move(internal),
                std::move(threshold),
                std::move(higher),
                std::move(lower));
        }
    };

    class TextureReverseCreator : public Creator<Texture>
    {
    public:

        std::string name() const override
        {
            return "reverse";
        }

        std::shared_ptr<Texture> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto common_params = init_common_params(params);
            auto internal = context.create<Texture>(params.child_group("internal"));
            return create_texture_reverse(common_params, std::move(internal));
        }
    };

} // namespace texture

void initialize_texture_factory(Factory<Texture> &factory)
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
