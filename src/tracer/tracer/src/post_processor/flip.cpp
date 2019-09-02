#include <agz/tracer/core/post_processor.h>

AGZ_TRACER_BEGIN

class Flip : public PostProcessor
{
    bool vertically_   = false;
    bool horizontally_ = false;

public:

    using PostProcessor::PostProcessor;

    static std::string description()
    {
        return R"___(
flip [PostProcessor]
    vertically   [0/1] (optional) enable vertical flipping (defaultly disabled)
    horizontally [0/1] (optional) enable horizontal flipping (defaultly disabled)
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &context) override
    {
        AGZ_HIERARCHY_TRY

        if(auto node = params.find_child("vertically"))
            vertically_ = node->as_value().as_int() != 0;
        if(auto node = params.find_child("horizontally"))
            horizontally_ = node->as_value().as_int() != 0;

        AGZ_HIERARCHY_WRAP("in initializing flip post processor")
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

AGZT_IMPLEMENTATION(PostProcessor, Flip, "flip")

AGZ_TRACER_END
