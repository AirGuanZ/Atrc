#include <agz/tracer/core/post_processor.h>

AGZ_TRACER_BEGIN

class Gamma : public PostProcessor
{
    real gamma_ = 1;

public:

    explicit Gamma(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

    static std::string description()
    {
        return R"___(
gamma [PostProcessor]
    gamma     [real] (optional) gamma value
    inv_gamma [real] (required only when 'gamma' is not specified) 1 / gamma value
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext&) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        if(auto node = params.find_child("gamma"))
            gamma_ = node->as_value().as_real();
        else if(node = params.find_child("inv_gamma"); node)
            gamma_ = 1 / node->as_value().as_real();

        AGZ_HIERARCHY_WRAP("in initializing gamma correction post processor")
    }

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

PostProcessor *create_gamma_corrector(
    real gamma,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<Gamma>(customed_flag);
    ret->initialize(gamma);
    return ret;
}

AGZT_IMPLEMENTATION(PostProcessor, Gamma, "gamma")

AGZ_TRACER_END
