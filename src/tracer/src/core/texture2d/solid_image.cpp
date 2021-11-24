#include <agz/tracer/core/texture2d.h>
#include <agz/tracer/core/texture3d.h>

AGZ_TRACER_BEGIN

class SolidImageTexture : public Texture2D
{
    RC<const Texture3D> tex3d_;

protected:

    FSpectrum sample_spectrum_impl(const Vec2 &uv) const noexcept override
    {
        return tex3d_->sample_spectrum({ uv.x, uv.y, 0 });
    }

public:

    explicit SolidImageTexture(RC<const Texture3D> tex3d)
        : tex3d_(std::move(tex3d))
    {
        
    }

    real sample_real(const EntityIntersection &inct) const noexcept override
    {
        return tex3d_->sample_real(inct.pos);
    }

    FSpectrum sample_spectrum(const EntityIntersection &inct) const noexcept override
    {
        return tex3d_->sample_spectrum(inct.pos);
    }

    int width() const noexcept override
    {
        return tex3d_->width();
    }

    int height() const noexcept override
    {
        return tex3d_->height();
    }
};

RC<Texture2D> create_solid_image_texture(RC<const Texture3D> tex3d)
{
    return newRC<SolidImageTexture>(std::move(tex3d));
}

AGZ_TRACER_END
