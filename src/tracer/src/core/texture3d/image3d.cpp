#include <agz/tracer/core/texture3d.h>
#include <agz/utility/texture.h>

AGZ_TRACER_BEGIN

// IMPROVE sampling performance

#define IF_ET(X)   if constexpr(std::is_same_v<ElemType, X>)
#define ELIF_ET(X) else if constexpr(std::is_same_v<ElemType, X>)
#define ELSE_ET    else

#define SWITCH_ET(ET_REAL, ET_UINT8, ET_SPEC, ET_UINT24) \
    do                                                   \
    {                                                    \
        IF_ET(real)                                      \
            ET_REAL                                      \
        ELIF_ET(uint8_t)                                 \
            ET_UINT8                                     \
        ELIF_ET(Spectrum)                                \
            ET_SPEC                                      \
        ELSE_ET                                          \
            ET_UINT24                                    \
    } while(false)

template<bool USE_LINEAR_INTERP, typename ElemType>
class ImageTexture3D : public Texture3D
{
    RC<const Image3D<ElemType>> data_;

    Spectrum max_spec_;
    real max_real_;

protected:

    real sample_real_impl(const Vec3 &uvw) const noexcept override
    {
        auto access_texel = [&](int x, int y, int z)
        {
            SWITCH_ET(
            {
                return data_->at(z, y, x);
            },
            {
                return data_->at(z, y, x) / real(255);
            },
            {
                return data_->at(z, y, x).r;
            },
            {
                return math::from_color3b<real>(data_->at(z, y, x)).r;
            });
        };

        if constexpr(USE_LINEAR_INTERP)
        {
            return texture::linear_sample3d(
                uvw, access_texel,
                data_->width(), data_->height(), data_->depth());
        }
        else
        {
            return texture::nearest_sample3d(
                uvw, access_texel,
                data_->width(), data_->height(), data_->depth());
        }
    }

    Spectrum sample_spectrum_impl(const Vec3 &uvw) const noexcept override
    {
        auto access_texel = [&](int x, int y, int z)
        {
            SWITCH_ET(
            {
                return Spectrum(data_->at(z, y, x));
            },
            {
                return Spectrum(data_->at(z, y, x) / real(255));
            },
            {
                return data_->at(z, y, x);
            },
            {
                return math::from_color3b<real>(data_->at(z, y, x));
            });
        };

        if constexpr(USE_LINEAR_INTERP)
        {
            return texture::linear_sample3d(
                uvw, access_texel,
                data_->width(), data_->height(), data_->depth());
        }
        else
        {
            return texture::nearest_sample3d(
                uvw, access_texel,
                data_->width(), data_->height(), data_->depth());
        }
    }

public:

    ImageTexture3D(
        const Texture3DCommonParams &common_params,
        RC<const Image3D<ElemType>> data)
    {
        init_common_params(common_params);
        data_ = std::move(data);

        max_spec_ = Spectrum(REAL_MIN);
        max_real_ = REAL_MIN;

        for(int z = 0; z < data_->depth(); ++z)
        {
            for(int y = 0; y < data_->height(); ++y)
            {
                for(int x = 0; x < data_->width(); ++x)
                {
                    SWITCH_ET(
                    {
                        max_real_ = (std::max)(max_real_, data_->at(z, y, x));
                    },
                    {
                        max_real_ = (std::max)(max_real_, data_->at(z, y, x) / real(255));
                    },
                    {
                        const Spectrum e = data_->at(z, y, x);
                        max_spec_.r = (std::max)(max_spec_.r, e.r);
                        max_spec_.g = (std::max)(max_spec_.g, e.g);
                        max_spec_.b = (std::max)(max_spec_.b, e.b);
                    },
                    {
                        const Spectrum e = math::from_color3b<real>(data_->at(z, y, x));
                        max_spec_.r = (std::max)(max_spec_.r, e.r);
                        max_spec_.g = (std::max)(max_spec_.g, e.g);
                        max_spec_.b = (std::max)(max_spec_.b, e.b);
                    });
                }
            }
        }

        SWITCH_ET(
            { max_spec_ = Spectrum(max_real_); },
            { max_spec_ = Spectrum(max_real_); },
            { max_real_ = max_spec_.r; },
            { max_real_ = max_spec_.r; });
    }

    int width() const noexcept override
    {
        return data_->width();
    }

    int height() const noexcept override
    {
        return data_->height();
    }

    int depth() const noexcept override
    {
        return data_->depth();
    }

    Spectrum max_spectrum() const noexcept override
    {
        return max_spec_;
    }

    real max_real() const noexcept override
    {
        return max_real_;
    }
};

RC<Texture3D> create_image3d(
    const Texture3DCommonParams &common_params,
    RC<const Image3D<real>> data,
    bool use_linear_sampler)
{
    if(use_linear_sampler)
    {
        return newRC<ImageTexture3D<true, real>>(
            common_params, std::move(data));
    }

    return newRC<ImageTexture3D<false, real>>(
        common_params, std::move(data));
}

RC<Texture3D> create_image3d(
    const Texture3DCommonParams &common_params,
    RC<const Image3D<uint8_t>> data,
    bool use_linear_sampler)
{
    if(use_linear_sampler)
    {
        return newRC<ImageTexture3D<true, uint8_t>>(
            common_params, std::move(data));
    }

    return newRC<ImageTexture3D<false, uint8_t>>(
        common_params, std::move(data));
}

RC<Texture3D> create_image3d(
    const Texture3DCommonParams &common_params,
    RC<const Image3D<Spectrum>> data,
    bool use_linear_sampler)
{
    if(use_linear_sampler)
    {
        return newRC<ImageTexture3D<true, Spectrum>>(
            common_params, std::move(data));
    }

    return newRC<ImageTexture3D<false, Spectrum>>(
        common_params, std::move(data));
}

RC<Texture3D> create_image3d(
    const Texture3DCommonParams &common_params,
    RC<const Image3D<math::color3b>> data,
    bool use_linear_sampler)
{
    if(use_linear_sampler)
    {
        return newRC<ImageTexture3D<true, math::color3b>>(
            common_params, std::move(data));
    }

    return newRC<ImageTexture3D<false, math::color3b>>(
        common_params, std::move(data));
}

AGZ_TRACER_END
