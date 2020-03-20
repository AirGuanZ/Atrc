#include <agz/factory/creator/post_processor_creators.h>
#include <agz/tracer/create/post_processor.h>

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

        RC<PostProcessor> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const real exposure = params.child_real("exposure");
            return create_aces_tone_mapper(exposure);
        }
    };

    class GammaCreator : public Creator<PostProcessor>
    {
    public:

        std::string name() const override
        {
            return "gamma";
        }

        RC<PostProcessor> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            real gamma;
            if(auto node = params.find_child("gamma"))
                gamma = node->as_value().as_real();
            else
                gamma = 1 / params.child_real("inv_gamma");
            return create_gamma_corrector(gamma);
        }
    };

#ifdef USE_OIDN

    class OIDNDenoiserCreator : public Creator<PostProcessor>
    {
    public:

        std::string name() const override
        {
            return "oidn_denoiser";
        }

        RC<PostProcessor> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const bool clamp_color = params.child_int_or("clamp", 0) != 0;
            return create_oidn_denoiser(clamp_color);
        }
    };

#endif

    class ResizeImageCreator : public Creator<PostProcessor>
    {
    public:

        std::string name() const override
        {
            return "resize";
        }

        RC<PostProcessor> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const Vec2i target_size = params.child_vec2i("size");
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

        RC<PostProcessor> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            std::string albedo_filename, normal_filename;

            if(auto node = params.find_child("albedo"))
                albedo_filename = context.path_mapper->map(node->as_value().as_str());
            if(auto node = params.find_child("normal"))
                normal_filename = context.path_mapper->map(node->as_value().as_str());
            
            return create_saving_gbuffer_to_png(
                std::move(albedo_filename),
                std::move(normal_filename));
        }
    };

    class SaveImgCreator : public Creator<PostProcessor>
    {
    public:

        std::string name() const override
        {
            return "save_to_img";
        }

        RC<PostProcessor> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            std::string filename = context.path_mapper->map(
                params.child_str("filename"));

            const bool open = params.child_int_or("open", 0) != 0;

            real gamma = 1;
            if(auto node = params.find_child_value("gamma"))
                gamma = node->as_real();
            else if(node = params.find_child_value("inv_gamma"); node)
                gamma = 1 / node->as_real();

            std::string ext;
            if(auto child_node = params.find_child_value("ext"))
                ext = child_node->as_str();
            else
            {
                if(stdstr::ends_with(filename, ".png"))
                    ext = "png";
                else if(stdstr::ends_with(filename, ".jpg"))
                    ext = "jpg";
                else if(stdstr::ends_with(filename, ".hdr"))
                    ext = "hdr";
                else
                    throw ObjectConstructionException(
                        "unknown image file format: " + filename);
            }

            return create_saving_to_img(
                std::move(filename), std::move(ext), open, gamma);
        }
    };

} // namespace post_processor

void initialize_post_processor_factory(Factory<PostProcessor> &factory)
{
    factory.add_creator(newBox<post_processor::ACESCreator>());
    factory.add_creator(newBox<post_processor::GammaCreator>());
#ifdef USE_OIDN
    factory.add_creator(newBox<post_processor::OIDNDenoiserCreator>());
#endif
    factory.add_creator(newBox<post_processor::ResizeImageCreator>());
    factory.add_creator(newBox<post_processor::SaveGBufferCreator>());
    factory.add_creator(newBox<post_processor::SaveImgCreator>());
}

AGZ_TRACER_FACTORY_END
