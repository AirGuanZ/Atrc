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
            reinterpret_cast<float*>(
                img.raw_data()), img.width(), img.height(), 0,
            reinterpret_cast<float*>(
                out_img.raw_data()), target_size_.x, target_size_.y, N, 0);
        img = std::move(out_img);
    }

public:

    explicit ImageResizer(const Vec2i &target_size) noexcept
        : target_size_(target_size)
    {
        
    }

    void process(RenderTarget &renderer_target) override
    {
        AGZ_INFO("resize image to ({}, {})", target_size_.x, target_size_.y);

        resize<Spectrum, 3>(renderer_target.image);
        if(renderer_target.albedo.is_available())
            resize<Spectrum, 3>(renderer_target.albedo);
        if(renderer_target.normal.is_available())
            resize<Vec3, 3>(renderer_target.normal);
        if(renderer_target.denoise.is_available())
            resize<real, 1>(renderer_target.denoise);
    }
};

RC<PostProcessor> create_img_resizer(const Vec2i &target_size)
{
    return newRC<ImageResizer>(target_size);
}

AGZ_TRACER_END
