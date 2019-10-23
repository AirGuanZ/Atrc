#include <avir.h>

#include <agz/tracer/core/post_processor.h>
#include <agz/tracer/utility/logger.h>

AGZ_TRACER_BEGIN

class ImageResizer : public PostProcessor
{
    Vec2i target_size_;

    template<typename T, int N>
    void resize(texture::texture2d_t<T> &img)
    {
        avir::CImageResizer image_resizer(8);
        texture::texture2d_t<T> out_img(target_size_.y, target_size_.x);
        image_resizer.resizeImage(
            reinterpret_cast<float*>(img.raw_data()), img.width(), img.height(), 0,
            reinterpret_cast<float*>(out_img.raw_data()), target_size_.x, target_size_.y, N, 0);
        img = std::move(out_img);
    }

public:

    explicit ImageResizer(const Vec2i &target_size) noexcept
        : target_size_(target_size)
    {
        
    }

    void process(texture::texture2d_t<Spectrum> &image, GBuffer &gbuffer) override
    {
        AGZ_LOG1("resize image to (", target_size_.x, ", ", target_size_.y, ")");

        resize<Spectrum, 3>(image);

        if(gbuffer.albedo)
            resize<Spectrum, 3>(*gbuffer.albedo);
        if(gbuffer.position)
            resize<Vec3, 3>(*gbuffer.position);
        if(gbuffer.normal)
            resize<Vec3, 3>(*gbuffer.normal);
        if(gbuffer.depth)
            resize<float, 1>(*gbuffer.depth);
        if(gbuffer.binary)
            resize<float, 1>(*gbuffer.binary);
    }
};

std::shared_ptr<PostProcessor> create_img_resizer(const Vec2i &target_size)
{
    return std::make_shared<ImageResizer>(target_size);
}

AGZ_TRACER_END
