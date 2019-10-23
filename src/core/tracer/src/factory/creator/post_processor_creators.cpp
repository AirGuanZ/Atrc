#include <agz/tracer/factory/creator/post_processor_creators.h>
#include <agz/tracer/factory/raw/post_processor.h>

AGZ_TRACER_FACTORY_BEGIN

namespace post_processor
{
    
    class ACESCreator : public Creator<PostProcessor>
    {
    public:

        std::string name() const override
        {
            return "aces";
        }

        std::shared_ptr<PostProcessor> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            real exposure = params.child_real("exposure");
            return create_aces_tone_mapper(exposure);
        }
    };

    class FlipCreator : public Creator<PostProcessor>
    {
    public:

        std::string name() const override
        {
            return "flip";
        }

        std::shared_ptr<PostProcessor> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            bool vertically = params.child_int_or("vertically", 0) != 0;
            bool horizontally = params.child_int_or("horizontally", 0) != 0;
            return create_film_flipper(vertically, horizontally);
        }
    };

    class GammaCreator : public Creator<PostProcessor>
    {
    public:

        std::string name() const override
        {
            return "gamma";
        }

        std::shared_ptr<PostProcessor> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            real gamma;
            if(auto node = params.find_child("gamma"))
                gamma = node->as_value().as_real();
            else
                gamma = 1 / params.child_real("inv_gamma");
            return create_gamma_corrector(gamma);
        }
    };

    class OIDNDenoiserCreator : public Creator<PostProcessor>
    {
    public:

        std::string name() const override
        {
            return "oidn_denoiser";
        }

        std::shared_ptr<PostProcessor> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            bool clamp_color = params.child_int_or("clamp", 0) != 0;
            return create_oidn_denoiser(clamp_color);
        }
    };

    class ResizeImageCreator : public Creator<PostProcessor>
    {
    public:

        std::string name() const override
        {
            return "resize";
        }

        std::shared_ptr<PostProcessor> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            Vec2i target_size = params.child_vec2i("size");
            return create_img_resizer(target_size);
        }
    };

    class SaveGBufferCreator : public Creator<PostProcessor>
    {
    public:

        std::string name() const override
        {
            return "save_gbuffer_to_png";
        }

        std::shared_ptr<PostProcessor> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            std::string albedo_filename, normal_filename, depth_filename, binary_filename;

            if(auto node = params.find_child("albedo"))
                albedo_filename = context.path_mapper->map(node->as_value().as_str());
            if(auto node = params.find_child("normal"))
                normal_filename = context.path_mapper->map(node->as_value().as_str());
            if(auto node = params.find_child("depth"))
                depth_filename = context.path_mapper->map(node->as_value().as_str());
            if(auto node = params.find_child("binary"))
                binary_filename = context.path_mapper->map(node->as_value().as_str());
            
            return create_saving_gbuffer_to_png(
                std::move(albedo_filename),
                std::move(normal_filename),
                std::move(depth_filename),
                std::move(binary_filename));
        }
    };

    class SaveImgCreator : public Creator<PostProcessor>
    {
    public:

        std::string name() const override
        {
            return "save_to_img";
        }

        std::shared_ptr<PostProcessor> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            std::string filename = context.path_mapper->map(params.child_str("filename"));
            bool open = params.child_int_or("open", 1) != 0;

            real gamma = 1;
            if(auto node = params.find_child_value("gamma"))
                gamma = node->as_real();
            else if(node = params.find_child_value("inv_gamma"); node)
                gamma = 1 / node->as_real();

            bool with_alpha_channel = params.child_int_or("with_alpha_channel", 0) != 0;
            
            std::string ext = params.child_str_or("ext", "png");

            return create_saving_to_img(
                std::move(filename), std::move(ext), open, gamma, with_alpha_channel);
        }
    };

    class SavePNGCreator : public SaveImgCreator
    {
    public:

        std::string name() const override
        {
            return "save_to_png";
        }
    };

} // namespace post_processor

void initialize_post_processor_factory(Factory<PostProcessor> &factory)
{
    factory.add_creator(std::make_unique<post_processor::ACESCreator>());
    factory.add_creator(std::make_unique<post_processor::FlipCreator>());
    factory.add_creator(std::make_unique<post_processor::GammaCreator>());
    factory.add_creator(std::make_unique<post_processor::OIDNDenoiserCreator>());
    factory.add_creator(std::make_unique<post_processor::ResizeImageCreator>());
    factory.add_creator(std::make_unique<post_processor::SaveGBufferCreator>());
    factory.add_creator(std::make_unique<post_processor::SaveImgCreator>());
    factory.add_creator(std::make_unique<post_processor::SavePNGCreator>());
}

AGZ_TRACER_FACTORY_END
