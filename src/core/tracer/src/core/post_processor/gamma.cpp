#include <agz/tracer/core/post_processor.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class Gamma : public PostProcessor
{
    real gamma_ = 1;

public:

    void initialize(real gamma)
    {
        AGZ_HIERARCHY_TRY

        gamma_ = gamma;
        if(gamma_ <= 0)
            throw ObjectConstructionException("invalid gamma value: " + std::to_string(gamma_));

        AGZ_HIERARCHY_WRAP("in initializing gamma correction post processor")
    }

    void process(texture::texture2d_t<Spectrum> &image, GBuffer &) override
    {
        for(int y = 0; y < image.height(); ++y)
        {
            for(int x = 0; x < image.width(); ++x)
            {
                image(y, x) = image(y, x).map([g = gamma_](real c)
                {
                    return std::pow(c, g);
                });
            }
        }
    }
};

std::shared_ptr<PostProcessor> create_gamma_corrector(
    real gamma)
{
    auto ret = std::make_shared<Gamma>();
    ret->initialize(gamma);
    return ret;
}

AGZ_TRACER_END
