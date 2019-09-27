#include <agz/tracer/core/post_processor.h>
#include <agz/tracer/utility/logger.h>
#include <agz/utility/file.h>
#include <agz/utility/image.h>
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

    explicit SaveToImage(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

    static std::string description()
    {
        return R"___(
save_to_img/save_to_png [PostProcessor]
    filename           [string] saving destination
    open               [0/1]    (optional) open saved file with default application (defaultly 1)
    gamma              [real]   (optional) gamma value
    inv_gamma          [real]   (optional) 1 / gamma value
    with_alpha_channel [0/1]    (optional; defaultly set to 0) use gbuffer::binary as alpha channel
    ext                [string] (optional; defaultly set to "png") save to png/jpg
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &context) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        std::string raw_filename = params.child_str("filename");
        filename_ = context.path_mgr->get(raw_filename);

        if(auto node = params.find_child("open"))
            open_after_saved_ = node->as_value().as_int() != 0;

        if(auto node = params.find_child("gamma"))
            gamma_ = node->as_value().as_real();
        else if(node = params.find_child("inv_gamma"); node)
            gamma_ = 1 / node->as_value().as_real();

        with_alpha_channel_ = params.child_int_or("with_alpha_channel", 0) != 0;

        if(auto node = params.find_child_value("ext"))
        {
            if(node->as_str() == "png" || node->as_str() == "jpg")
                save_ext_  = node->as_str();
            else
                throw ObjectConstructionException("unsupported image extension: " + node->as_str());
        }

        AGZ_HIERARCHY_WRAP("in initializing save_to_png post processor")
    }

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

PostProcessor *create_saving_to_img(
    std::string filename, std::string ext,
    bool open, real gamma, bool with_alpha_channel,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<SaveToImage>(customed_flag);
    ret->initialize(filename, ext, open, gamma, with_alpha_channel);
    return ret;
}

using SaveToPNG = SaveToImage;

AGZT_IMPLEMENTATION(PostProcessor, SaveToImage, "save_to_img")
AGZT_IMPLEMENTATION(PostProcessor, SaveToPNG, "save_to_png")

AGZ_TRACER_END
