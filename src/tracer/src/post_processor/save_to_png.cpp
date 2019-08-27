#include <agz/tracer/core/post_processor.h>
#include <agz/tracer/utility/logger.h>
#include <agz/utility/file.h>
#include <agz/utility/image.h>
#include <agz/utility/system.h>

AGZ_TRACER_BEGIN

class SaveToPNG : public PostProcessor
{
    std::string filename_;
    bool open_after_saved_ = true;
    real gamma_ = 1;

public:

    using PostProcessor::PostProcessor;

    static std::string description()
    {
        return R"___(
save_to_png [PostProcessor]
    filename [string] saving destination
    open     [0/1] (optional) open saved file with default application (defaultly 1)
    gamma     [real] (optional) gamma value
    inv_gamma [real] (optional) 1 / gamma value
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &context) override
    {
        AGZ_HIERARCHY_TRY

        std::string raw_filename = params.child_str("filename");
        filename_ = context.path_mgr->get(raw_filename);

        if(auto node = params.find_child("open"))
            open_after_saved_ = node->as_value().as_int() != 0;

        if(auto node = params.find_child("gamma"))
            gamma_ = node->as_value().as_real();
        else if(node = params.find_child("inv_gamma"); node)
            gamma_ = 1 / node->as_value().as_real();

        AGZ_HIERARCHY_WRAP("in initializing save_to_png post processor")
    }

    void process(texture::texture2d_t<Spectrum> &image, GBuffer &) override
    {
        auto img_u8 = image.get_data().map([gamma = gamma_](const Spectrum &s)
        {
            return s.map([gamma = gamma](real c)
            {
                return static_cast<uint8_t>(math::clamp<real>(std::pow(c, gamma), 0, 1) * 255);
            });
        });

        file::create_directory_for_file(filename_);

        AGZ_LOG0("saving image to ", filename_);
        img::save_rgb_to_png_file(filename_, img_u8);

        if(open_after_saved_)
            sys::open_with_default_app(filename_);
    }
};

AGZT_IMPLEMENTATION(PostProcessor, SaveToPNG, "save_to_png")

AGZ_TRACER_END
