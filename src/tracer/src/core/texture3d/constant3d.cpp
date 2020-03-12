#include <agz/tracer/core/texture3d.h>

AGZ_TRACER_BEGIN

class Constant3D : public Texture3D
{
    Spectrum texel_;

public:

    Constant3D(const Texture3DCommonParams &common_params, const Spectrum &texel)
        : texel_(texel)
    {
        init_common_params(common_params);

        if(inv_gamma_ != 1)
        {
            for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
                texel_[i] = std::pow(texel_[i], inv_gamma_);
        }
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

    Spectrum min_spectrum() const noexcept override
    {
        return texel_;
    }

    real max_real() const noexcept override
    {
        return texel_.r;
    }

    real min_real() const noexcept override
    {
        return texel_.r;
    }

    Spectrum sample_spectrum(const Vec3 &uvw) const noexcept override
    {
        return texel_;
    }

    real sample_real(const Vec3 &uvw) const noexcept override
    {
        return texel_.r;
    }
};

RC<Texture3D> create_constant3d_texture(
    const Texture3DCommonParams &common_params,
    const Spectrum &texel)
{
    return newRC<Constant3D>(common_params, texel);
}

AGZ_TRACER_END
