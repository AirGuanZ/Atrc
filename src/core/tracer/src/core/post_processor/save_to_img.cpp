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

    std::string save_ext_ = "png";

public:

    SaveToImage(std::string filename, std::string ext, bool open, real gamma)
    {
        AGZ_HIERARCHY_TRY

        filename_ = std::move(filename);
        save_ext_ = std::move(ext);

        if(save_ext_ != "png" && save_ext_ != "jpg")
            throw ObjectConstructionException("unsupported image extension: " + save_ext_);

        open_after_saved_ = open;
        gamma_ = gamma;

        if(gamma_ <= 0)
            throw ObjectConstructionException("invalid gamma value: " + std::to_string(gamma_));

        AGZ_HIERARCHY_WRAP("in initializing save_to_png post processor")
    }

    void process(RenderTarget &render_target) override
    {
        file::create_directory_for_file(filename_);

        AGZ_INFO("saving image to {}", filename_);

        const auto imgu8 = render_target.image.get_data().map([gamma = gamma_](const Spectrum &s)
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

        if(open_after_saved_)
            sys::open_with_default_app(filename_);
    }
};

std::shared_ptr<PostProcessor> create_saving_to_img(
    std::string filename, std::string ext,
    bool open, real gamma)
{
    return std::make_shared<SaveToImage>(filename, ext, open, gamma);
}

AGZ_TRACER_END
