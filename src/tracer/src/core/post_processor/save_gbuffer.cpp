#include <agz/tracer/core/post_processor.h>
#include <agz/tracer/utility/logger.h>
#include <agz/utility/file.h>
#include <agz/utility/image.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class SaveGBufferToPNG : public PostProcessor
{
    std::string albedo_filename_;
    std::string normal_filename_;

    static void save_albedo(const std::string &filename, Image2D<Spectrum> &albedo)
    {
        file::create_directory_for_file(filename);
        AGZ_INFO("saving gbuffer::albedo to {}", filename);
        img::save_rgb_to_png_file(filename, albedo.flip_vertically().get_data().map(math::to_color3b<real>));
    }

    static void save_normal(const std::string &filename, Image2D<Vec3> &normal)
    {
        file::create_directory_for_file(filename);
        texture::texture2d_t<Spectrum> imgf(normal.height(), normal.width());
        for(int y = 0; y < imgf.height(); ++y)
        {
            for(int x = 0; x < imgf.width(); ++x)
            {
                Spectrum nor(normal(y, x).x, normal(y, x).y, normal(y, x).z);
                imgf(y, x) = (Spectrum(1) + nor) * real(0.5);
            }
        }

        AGZ_INFO("saving gbuffer::normal to {}", filename);
        img::save_rgb_to_png_file(filename, imgf.flip_vertically().get_data().map(math::to_color3b<real>));
    }

public:

    SaveGBufferToPNG(
        std::string albedo_filename,
        std::string normal_filename)
    {
        albedo_filename_ = std::move(albedo_filename);
        normal_filename_ = std::move(normal_filename);
    }

    void process(RenderTarget &render_target) override
    {
        if(!albedo_filename_.empty() && render_target.albedo.is_available())
            save_albedo(albedo_filename_, render_target.albedo);
        if(!normal_filename_.empty() && render_target.normal.is_available())
            save_normal(normal_filename_, render_target.normal);
    }
};

std::shared_ptr<PostProcessor> create_saving_gbuffer_to_png(
    std::string albedo_filename,
    std::string normal_filename)
{
    return std::make_shared<SaveGBufferToPNG>(
        std::move(albedo_filename),
        std::move(normal_filename));
}

AGZ_TRACER_END
