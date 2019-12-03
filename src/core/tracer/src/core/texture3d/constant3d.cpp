#include <agz/tracer/core/texture3d.h>

AGZ_TRACER_BEGIN

class Constant3D : public Texture3D
{
    Spectrum texel_;

protected:

    Spectrum sample_spectrum_impl(const Vec3 &uvw) const noexcept override
    {
        return texel_;
    }

public:

    Constant3D(const Texture3DCommonParams &common_params, const Spectrum &texel)
        : texel_(texel)
    {
        init_common_params(common_params);
    }

    int width() const noexcept override
    {
        return 1;
    }

    int height() const noexcept override
    {
        return 1;
    }

    int depth() const noexcept override
    {
        return 1;
    }

    Spectrum max_spectrum() const noexcept override
    {
        return texel_;
    }

    real max_real() const noexcept override
    {
        return texel_.r;
    }
};

std::shared_ptr<Texture3D> create_constant3d_texture(
    const Texture3DCommonParams &common_params,
    const Spectrum &texel)
{
    return std::make_shared<Constant3D>(common_params, texel);
}

AGZ_TRACER_END
