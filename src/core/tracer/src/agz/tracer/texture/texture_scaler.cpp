#include <agz/tracer/core/texture.h>

AGZ_TRACER_BEGIN

class TextureScaler : public Texture
{
    Spectrum scale_;
    const Texture *internal_ = nullptr;

public:

    explicit TextureScaler(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

    static std::string description()
    {
        return R"___(
scaler [Texture]
    scale    [Spectrum] scaling ratio
    internal [Texture]  scaled texture
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        init_common_params(params);
        
        scale_    = params.child_spectrum("scale");
        internal_ = TextureFactory.create(params.child_group("internal"), init_ctx);

        AGZ_HIERARCHY_WRAP("in initializing texture scaler object")
    }

    void initialize(const Spectrum &scale, const Texture *internal)
    {
        scale_ = scale;
        internal_ = internal;
    }

    Spectrum sample_spectrum(const Vec2 &uv) const noexcept override
    {
        return scale_ * internal_->sample_spectrum(uv);
    }

    int width() const noexcept override
    {
        return internal_->width();
    }

    int height() const noexcept override
    {
        return internal_->height();
    }
};

Texture *create_texture_scaler(
    const Spectrum &scale, const Texture *internal,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<TextureScaler>(customed_flag);
    ret->initialize(scale, internal);
    return ret;
}

AGZT_IMPLEMENTATION(Texture, TextureScaler, "scale")

AGZ_TRACER_END
