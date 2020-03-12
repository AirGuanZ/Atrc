#include <memory>
#include <unordered_map>

#include <agz/tracer/core/texture2d.h>
#include <agz/tracer/utility/logger.h>
#include <agz/utility/misc.h>
#include <agz/utility/texture.h>

AGZ_TRACER_BEGIN

class HDRTexture : public Texture2D
{
    RC<const Image2D<math::color3f>> data_;

    static Spectrum nearest_sample_impl(
        const texture::texture2d_t<math::color3f> *data, const Vec2 &uv) noexcept
    {
        const auto tex = [&t = *data](int x, int y) { return t(y, x); };
        return texture::nearest_sample2d(uv, tex, data->width(), data->height());
    }

    static Spectrum linear_sample_impl(
        const texture::texture2d_t<math::color3f> *data, const Vec2 &uv) noexcept
    {
        const auto tex = [&t = *data](int x, int y) { return t(y, x); };
        return texture::linear_sample2d(uv, tex, data->width(), data->height());
    }

    using SampleImplFuncPtr = Spectrum(*)(
        const texture::texture2d_t<math::color3f>*, const Vec2&);
    SampleImplFuncPtr sample_impl_ = linear_sample_impl;

protected:
    
    Spectrum sample_spectrum_impl(const Vec2 &uv) const noexcept override
    {
        return sample_impl_(data_.get(), uv.saturate());
    }

public:

    HDRTexture(
        const Texture2DCommonParams &common_params,
        RC<const Image2D<math::color3f>> data,
        const std::string &sampler)
    {
        AGZ_HIERARCHY_TRY

        init_common_params(common_params);

        data_ = std::move(data);

        if(sampler == "nearest")
            sample_impl_ = nearest_sample_impl;
        else if(sampler == "linear")
            sample_impl_ = linear_sample_impl;
        else
            throw ObjectConstructionException("invalid sample method");

        AGZ_HIERARCHY_WRAP("in initializing hdr texture object")
    }

    int width() const noexcept override
    {
        return data_->width();
    }

    int height() const noexcept override
    {
        return data_->height();
    }
};

RC<Texture2D> create_hdr_texture(
    const Texture2DCommonParams &common_params,
    RC<const Image2D<math::color3f>> data, const std::string &sampler)
{
    return newRC<HDRTexture>(common_params, data, sampler);
}

AGZ_TRACER_END
