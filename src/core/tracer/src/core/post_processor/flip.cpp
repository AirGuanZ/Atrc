#include <agz/tracer/core/post_processor.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class Flip : public PostProcessor
{
    bool vertically_   = false;
    bool horizontally_ = false;

public:

    void initialize(bool vertically, bool horizontally)
    {
        vertically_ = vertically;
        horizontally_ = horizontally;
    }

    void process(texture::texture2d_t<Spectrum> &image, GBuffer &gbuffer) override
    {
        if(vertically_)
        {
            texture::texture2d_t<Spectrum> t(image.height(), image.width());
            GBuffer tgb = gbuffer;
            for(int y = 0; y < t.height(); ++y)
            {
                for(int x = 0; x < t.width(); ++x)
                {
                    int ty = t.height() - 1 - y;
                    t(y, x) = image(ty, x);
                    tgb.set(y, x, gbuffer.get(ty, x));
                }
            }
            image = std::move(t);
            gbuffer = std::move(tgb);
        }

        if(horizontally_)
        {
            texture::texture2d_t<Spectrum> t(image.height(), image.width());
            GBuffer tgb = gbuffer;
            for(int y = 0; y < t.height(); ++y)
            {
                for(int x = 0; x < t.width(); ++x)
                {
                    int tx = t.width() - 1 - x;
                    t(y, x) = image(y, tx);
                    tgb.set(y, x, gbuffer.get(y, tx));
                }
            }
            image = std::move(t);
            gbuffer = std::move(tgb);
        }
    }
};

std::shared_ptr<PostProcessor> create_film_flipper(
    bool vertically, bool horizontally)
{
    auto ret = std::make_shared<Flip>();
    ret->initialize(vertically, horizontally);
    return ret;
}

AGZ_TRACER_END
