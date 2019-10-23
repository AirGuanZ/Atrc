#include <agz/tracer/core/post_processor.h>
#include <agz/tracer/utility/logger.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class ACESToneMapper : public PostProcessor
{
    static real aces_curve(real x) noexcept
    {
        real tA = real(2.51);
        real tB = real(0.03);
        real tC = real(2.43);
        real tD = real(0.59);
        real tE = real(0.14);
        return math::clamp((x * (tA * x + tB)) / (x * (tC * x + tD) + tE), real(0), real(1));
    }

    static Spectrum avg_lum(const ImageBuffer &img)
    {
        Spectrum sum;
        for(int y = 0; y < img.height(); ++y)
        {
            for(int x = 0; x < img.width(); ++x)
            {
                sum.r += std::log(real(0.01) + math::clamp<real>(img(y, x).r, 0, 4));
                sum.g += std::log(real(0.01) + math::clamp<real>(img(y, x).g, 0, 4));
                sum.b += std::log(real(0.01) + math::clamp<real>(img(y, x).b, 0, 4));
            }
        }

        return sum.map([&](real x)
        {
            real t = std::exp(x / (img.width() * img.height()));
            return t;
        });
    }

    real exposure_ = 10;

public:

    void initialize(real exposure)
    {
        AGZ_HIERARCHY_TRY

        exposure_ = exposure;
        if(exposure_ < 0)
            throw ObjectConstructionException("invalid exposure value");

        AGZ_HIERARCHY_WRAP("in initializing ACES tone mapper")
    }

    void process(ImageBuffer &image, GBuffer&) override
    {
        AGZ_LOG1("aces tone mapping");

        for(int y = 0; y < image.height(); ++y)
        {
            for(int x = 0; x < image.width(); ++x)
            {
                auto &pixel = image(y, x);
                pixel.r = aces_curve(pixel.r * exposure_);
                pixel.g = aces_curve(pixel.g * exposure_);
                pixel.b = aces_curve(pixel.b * exposure_);
            }
        }
    }
};

std::shared_ptr<PostProcessor> create_aces_tone_mapper(
    real exposure)
{
    auto ret = std::make_shared<ACESToneMapper>();
    ret->initialize(exposure);
    return ret;
}

AGZ_TRACER_END
