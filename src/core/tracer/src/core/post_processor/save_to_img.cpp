#include <agz/tracer/core/post_processor.h>
#include <agz/tracer/utility/logger.h>
#include <agz/utility/file.h>
#include <agz/utility/image.h>
#include <agz/utility/misc.h>
#include <agz/utility/system.h>

AGZ_TRACER_BEGIN

class SaveToImage : public PostProcessor
{
    std::string filename_;
    bool open_after_saved_ = true;
    real gamma_ = 1;
    bool with_alpha_channel_ = false;

    std::string save_ext_ = "png";

public:

    void initialize(std::string filename, std::string ext, bool open, real gamma, bool with_alpha_channel)
    {
        AGZ_HIERARCHY_TRY

        filename_ = std::move(filename);
        save_ext_ = std::move(ext);

        if(save_ext_ != "png" && save_ext_ != "jpg")
            throw ObjectConstructionException("unsupported image extension: " + save_ext_);

        open_after_saved_ = open;
        gamma_ = gamma;
        with_alpha_channel_ = with_alpha_channel;

        if(gamma_ <= 0)
            throw ObjectConstructionException("invalid gamma value: " + std::to_string(gamma_));

        AGZ_HIERARCHY_WRAP("in initializing save_to_png post processor")
    }

    void process(texture::texture2d_t<Spectrum> &image, GBuffer &gbuffer) override
    {

        file::create_directory_for_file(filename_);

        AGZ_LOG0("saving image to ", filename_);

        if(!with_alpha_channel_ || !gbuffer.binary)
        {
            auto imgu8 = image.get_data().map([gamma = gamma_](const Spectrum &s)
            {
                return s.map([gamma = gamma](real c)
                {
                    return static_cast<uint8_t>(math::clamp<real>(std::pow(c, gamma), 0, 1) * 255);
                });
            });
            if(save_ext_ == "png")
                img::save_rgb_to_png_file(filename_, imgu8);
            else if(save_ext_ == "jpg")
                img::save_rgb_to_jpg_file(filename_, imgu8);
        }
        else
        {
            texture::texture2d_t<math::color4b> imgu8(image.height(), image.width());
            for(int y = 0; y < imgu8.height(); ++y)
            {
                for(int x = 0; x < imgu8.width(); ++x)
                {
                    auto rgb = image(y, x).map([gamma = gamma_](real c)
                    {
                        return static_cast<uint8_t>(math::clamp<real>(std::pow(c, gamma), 0, 1) * 255);
                    });
                    imgu8(y, x).r = rgb.r;
                    imgu8(y, x).g = rgb.g;
                    imgu8(y, x).b = rgb.b;

                    real af = math::clamp<real>(gbuffer.binary->at(y, x), 0, 1);
                    imgu8(y, x).a = static_cast<uint8_t>(af * 255);
                }
            }
            if(save_ext_ == "png")
                img::save_rgba_to_png_file(filename_, imgu8.get_data());
            else if(save_ext_ == "jpg")
                img::save_rgba_to_jpg_file(filename_, imgu8.get_data());
        }

        if(open_after_saved_)
            sys::open_with_default_app(filename_);
    }
};

std::shared_ptr<PostProcessor> create_saving_to_img(
    std::string filename, std::string ext,
    bool open, real gamma, bool with_alpha_channel)
{
    auto ret = std::make_shared<SaveToImage>();
    ret->initialize(filename, ext, open, gamma, with_alpha_channel);
    return ret;
}

AGZ_TRACER_END
