#include <memory>
#include <unordered_map>

#include <agz/tracer/core/texture2d.h>
#include <agz/tracer/utility/logger.h>
#include <agz/utility/image.h>
#include <agz/utility/misc.h>
#include <agz/utility/texture.h>

AGZ_TRACER_BEGIN

class ImageTexture : public Texture2D
{
    RC<const Image2D<math::color3b>> data_;

    static Spectrum nearest_sample_impl(
        const texture::texture2d_t<math::color3b> *data, const Vec2 &uv) noexcept
    {
        const auto tex = [&t = *data](int x, int y)
            { return math::from_color3b<real>(t(y, x)); };
        return texture::nearest_sample2d(uv, tex, data->width(), data->height());
    }

    static Spectrum linear_sample_impl(
        const texture::texture2d_t<math::color3b> *data, const Vec2 &uv) noexcept
    {
        const auto tex = [&t = *data](int x, int y)
            { return math::from_color3b<real>(t(y, x)); };
        return texture::linear_sample2d(uv, tex, data->width(), data->height());
    }

    using SampleImplFuncPtr = Spectrum(*)(
        const texture::texture2d_t<math::color3b>*, const Vec2&);
    SampleImplFuncPtr sample_impl_ = linear_sample_impl;

protected:

    Spectrum sample_spectrum_impl(const Vec2 &uv) const noexcept override
    {
        return sample_impl_(data_.get(), uv.saturate());
    }

public:

    ImageTexture(
        const Texture2DCommonParams &common_params,
        RC<const Image2D<math::color3b>> data,
        const std::string &sampler)
    {
        init_common_params(common_params);

        assert(data && data->is_available());
        data_ = std::move(data);


        if(sampler == "nearest")
            sample_impl_ = nearest_sample_impl;
        else if(sampler == "linear")
            sample_impl_ = linear_sample_impl;
        else
            throw ObjectConstructionException("invalid sample method");
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

RC<Texture2D> create_image_texture(
    const Texture2DCommonParams &common_params,
    RC<const Image2D<math::color3b>> data, const std::string &sampler)
{
    return newRC<ImageTexture>(common_params, std::move(data), sampler);
}

AGZ_TRACER_END
