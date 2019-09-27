#include <memory>
#include <unordered_map>

#include <agz/tracer/core/texture.h>
#include <agz/tracer/utility/logger.h>
#include <agz/utility/image.h>
#include <agz/utility/texture.h>

AGZ_TRACER_BEGIN

class ImageTexture : public Texture
{
    const texture::texture2d_t<math::color3b> *data_ = nullptr;

    static Spectrum nearest_sample_impl(const texture::texture2d_t<math::color3b> *data, const Vec2 &uv) noexcept
    {
        auto tex = [&t = *data](int x, int y) { return math::from_color3b<real>(t(y, x)); };
        return texture::nearest_sample(uv, tex, data->width(), data->height());
    }

    static Spectrum linear_sample_impl(const texture::texture2d_t<math::color3b> *data, const Vec2 &uv) noexcept
    {
        auto tex = [&t = *data](int x, int y) { return math::from_color3b<real>(t(y, x)); };
        return texture::linear_sample(uv, tex, data->width(), data->height());
    }

    using SampleImplFuncPtr = Spectrum(*)(const texture::texture2d_t<math::color3b>*, const Vec2&);
    SampleImplFuncPtr sample_impl_ = linear_sample_impl;

protected:

    Spectrum sample_spectrum_impl(const Vec2 &uv) const noexcept override
    {
        return sample_impl_(data_, uv.saturate());
    }

public:

    explicit ImageTexture(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

    static std::string description()
    {
        return R"___(
image [Texture]
    filename [string] image filename
    inv_u    [0/1] (optional; defaultly set to 0) inverse u coord
    inv_v    [0/1] (optional; defaultly set to 0) inverse v coord
    swap_uv  [0/1] (optional; defaultly set to 0) swap uv coord; executed before inv_u/v 
    wrap_u   [clamp/repeat/mirror] (optional; defaultly set to clamp) texcoord u wrapper
    wrap_v   [clamp/repeat/mirror] (optional; defaultly set to clamp) texcoord v wrapper
    sample   [string] sample strategy (nearest or linear)
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        init_common_params(params);

        std::string raw_filename = params.child_str("filename");
        std::string filename = init_ctx.path_mgr->get(raw_filename);

        static std::unordered_map<std::string, std::unique_ptr<texture::texture2d_t<math::color3b>>> filename2tex_;

        if(auto it = filename2tex_.find(filename); it != filename2tex_.end())
        {
            data_ = it->second.get();
            AGZ_LOG2("use cached image texture of ", filename);
        }
        else
        {
            auto data = img::load_rgb_from_file(filename);
            if(!data.is_available())
                throw ObjectConstructionException("failed to load texture from " + filename);
            AGZ_LOG2("load image from ", filename);

            filename2tex_[filename] = std::make_unique<texture::texture2d_t<math::color3b>>(std::move(data));
            data_ = filename2tex_[filename].get();
            assert(data_->is_available());
        }

        if(auto val = params.find_child_value("sample"))
        {
            if(val->as_str() == "nearest")
                sample_impl_ = nearest_sample_impl;
            else if(val->as_str() == "linear")
                sample_impl_ = linear_sample_impl;
            else
                throw ObjectConstructionException("invalid sample method");
        }

        AGZ_HIERARCHY_WRAP("in initializing image texture object")
    }

    void initialize(
        const TextureCommonParams &common_params,
        const std::string &filename,
        const std::string &sampler)
    {
        AGZ_HIERARCHY_TRY

        init_common_params(common_params);

        static std::unordered_map<std::string, std::unique_ptr<texture::texture2d_t<math::color3b>>> filename2tex_;

        if(auto it = filename2tex_.find(filename); it != filename2tex_.end())
        {
            data_ = it->second.get();
            AGZ_LOG2("use cached image texture of ", filename);
        }
        else
        {
            auto data = img::load_rgb_from_file(filename);
            if(!data.is_available())
                throw ObjectConstructionException("failed to load texture from " + filename);
            AGZ_LOG2("load image from ", filename);

            filename2tex_[filename] = std::make_unique<texture::texture2d_t<math::color3b>>(std::move(data));
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

Texture *create_image_texture(
    const TextureCommonParams &common_params,
    const std::string &filename, const std::string &sampler,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<ImageTexture>(customed_flag);
    ret->initialize(common_params, filename, sampler);
    return ret;
}

AGZT_IMPLEMENTATION(Texture, ImageTexture, "image")

AGZ_TRACER_END
