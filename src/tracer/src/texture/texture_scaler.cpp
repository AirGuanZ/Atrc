#include <agz/tracer/core/texture.h>

AGZ_TRACER_BEGIN

class TextureScaler : public Texture
{
    Spectrum scale_;
    const Texture *internal_ = nullptr;

public:

    using Texture::Texture;

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

        init_transform(params);
        
        scale_    = params.child_spectrum("scale");
        internal_ = TextureFactory.create(params.child_group("internal"), init_ctx);

        AGZ_HIERARCHY_WRAP("in initializing texture scaler object")
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

    Spectrum fetch_spectrum(int x, int y) const noexcept override
    {
        return scale_ * internal_->fetch_spectrum(x, y);
    }

    real fetch_real(int x, int y) const noexcept override
    {
        return (scale_ * internal_->fetch_real(x, y)).lum();
    }
};

AGZT_IMPLEMENTATION(Texture, TextureScaler, "scale")

AGZ_TRACER_END
