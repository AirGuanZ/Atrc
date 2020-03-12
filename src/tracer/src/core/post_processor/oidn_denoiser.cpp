#ifdef USE_OIDN

#include <agz/tracer/core/post_processor.h>
#include <agz/tracer/utility/logger.h>
#include <agz/utility/misc.h>

#include <OpenImageDenoise/oidn.hpp>

AGZ_TRACER_BEGIN

class OIDNDenoiser : public PostProcessor
{
    bool clamp_color_ = false;

public:

    explicit OIDNDenoiser(bool clamp_color)
    {
        clamp_color_ = clamp_color;
    }

    void process(RenderTarget &render_target) override
    {
        AGZ_INFO("oidn denoising");

        oidn::DeviceRef device = oidn::newDevice();
        device.commit();

        auto &image = render_target.image;
        Image2D<Spectrum> output(image.height(), image.width());

        oidn::FilterRef filter = device.newFilter("RT");

        auto clamped_data = image.get_data();
        if(clamp_color_)
        {
            clamped_data = clamped_data.map(
                [](const Spectrum &c) { return c.clamp(0, 1); });
        }
        filter.setImage(
            "color", clamped_data.raw_data(),
            oidn::Format::Float3, image.width(), image.height());

        Image2D<Spectrum>::data_t clamped_albedo;
        if(render_target.albedo.is_available())
        {
            clamped_albedo = render_target.albedo.get_data().map(
                [](const Spectrum &a) { return a.clamp(0, 1); });
            filter.setImage(
                "albedo", clamped_albedo.raw_data(),
                oidn::Format::Float3, image.width(), image.height());
        }

        Image2D<Vec3>::data_t clamped_normal;
        if(render_target.normal.is_available())
        {
            clamped_normal = render_target.normal.get_data().map(
                [](const Vec3 &n)
            {
                if(!n)
                    return Vec3(1, 0, 0);
                return n.normalize().clamp(-1, 1);
            });
            filter.setImage(
                "normal", clamped_normal.raw_data(),
                oidn::Format::Float3, image.width(), image.height());
        }

        filter.setImage(
            "output", output.get_data().raw_data(),
            oidn::Format::Float3, image.width(), image.height());
        filter.set("hdr", true);
        filter.commit();
        filter.execute();

        if(render_target.denoise.is_available())
        {
            for(int y = 0; y < output.height(); ++y)
            {
                for(int x = 0; x < output.width(); ++x)
                {
                    if(render_target.denoise(y, x) < real(0.8))
                        output.at(y, x) = clamped_data(y, x);
                }
            }
        }

        const char* err;
        if(device.getError(err) != oidn::Error::None)
            AGZ_INFO("oidn_denoiser error: {}", err);
        else
            image = std::move(output);
    }
};

RC<PostProcessor> create_oidn_denoiser(
    bool clamp_color)
{
    return newRC<OIDNDenoiser>(clamp_color);
}

AGZ_TRACER_END

#endif // #ifdef USE_OIDN
