#include <memory>
#include <unordered_map>

#include <agz/tracer/core/texture2d.h>
#include <agz/tracer/utility/logger.h>
#include <agz/utility/image.h>
#include <agz/utility/misc.h>
#include <agz/utility/texture.h>

AGZ_TRACER_BEGIN

class HDRTexture : public Texture2D
{
    const texture::texture2d_t<math::color3f> *data_ = nullptr;

    static Spectrum nearest_sample_impl(const texture::texture2d_t<math::color3f> *data, const Vec2 &uv) noexcept
    {
        auto tex = [&t = *data](int x, int y) { return t(y, x); };
        return texture::nearest_sample(uv, tex, data->width(), data->height());
    }

    static Spectrum linear_sample_impl(const texture::texture2d_t<math::color3f> *data, const Vec2 &uv) noexcept
    {
        auto tex = [&t = *data](int x, int y) { return t(y, x); };
        return texture::linear_sample(uv, tex, data->width(), data->height());
    }

    using SampleImplFuncPtr = Spectrum(*)(const texture::texture2d_t<math::color3f>*, const Vec2&);
    SampleImplFuncPtr sample_impl_ = linear_sample_impl;

protected:
    
    Spectrum sample_spectrum_impl(const Vec2 &uv) const noexcept override
    {
        return sample_impl_(data_, uv.saturate());
    }

public:

    void initialize(
        const Texture2DCommonParams &common_params,
        const std::string &filename,
        const std::string &sampler)
    {
        AGZ_HIERARCHY_TRY

        init_common_params(common_params);

        static std::unordered_map<std::string, std::unique_ptr<texture::texture2d_t<math::color3f>>> filename2tex_;

        if(auto it = filename2tex_.find(filename); it != filename2tex_.end())
        {
            data_ = it->second.get();
            AGZ_LOG2("use cached hdr texture of ", filename);
        }
        else
        {
            auto data = img::load_rgb_from_hdr_file(filename);
            if(!data.is_available())
                throw ObjectConstructionException("failed to load texture from " + filename);
            AGZ_LOG2("load hdr from ", filename);

            filename2tex_[filename] = std::make_unique<texture::texture2d_t<math::color3f>>(std::move(data));
            data_ = filename2tex_[filename].get();
            assert(data_->is_available());
        }

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

std::shared_ptr<Texture2D> create_hdr_texture(
    const Texture2DCommonParams &common_params,
    const std::string &filename, const std::string &sampler)
{
    auto ret = std::make_shared<HDRTexture>();
    ret->initialize(common_params, filename, sampler);
    return ret;
}

AGZ_TRACER_END
