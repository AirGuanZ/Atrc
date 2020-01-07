#pragma once

#include <agz/tracer/core/film_filter.h>
#include <agz/utility/texture.h>

AGZ_TRACER_BEGIN

namespace img_buf_impl
{
    template<bool WITH_VALUE> struct ValueBuffer { void init(int w, int h) { } };
    template<> struct ValueBuffer<true> { Image2D<Spectrum> value; void init(int w, int h) { value.initialize(h, w); } };

    template<bool WITH_WEIGHT> struct WeightBuffer { void init(int w, int h) { } };
    template<> struct WeightBuffer<true> { Image2D<real> weight; void init(int w, int h) { weight.initialize(h, w); } };

    template<bool WITH_ALBEDO> struct AlbedoBuffer { void init(int w, int h) { } };
    template<> struct AlbedoBuffer<true> { Image2D<Spectrum> albedo; void init(int w, int h) { albedo.initialize(h, w); } };

    template<bool WITH_NORMAL> struct NormalBuffer { void init(int w, int h) { } };
    template<> struct NormalBuffer<true> { Image2D<Vec3> normal; void init(int w, int h) { normal.initialize(h, w); } };

    template<bool WITH_DENOISE> struct DenoiseBuffer { void init(int w, int h) { } };
    template<> struct DenoiseBuffer<true> { Image2D<real> denoise; void init(int w, int h) { denoise.initialize(h, w); } };

}

/**
 * @brief 用于存储中间结果的image buffer的标准模板
 *
 * 通过模板参数控制其是否拥有指定的成员：
 * Image2D<Spectrum> value
 * Image2D<real>     weight
 * Image2D<Spectrum> albedo
 * Image2D<Vec3>     normal
 * Image2D<real>     denoise
 */
template<bool WITH_VALUE, bool WITH_WEIGHT, bool WITH_ALBEDO, bool WITH_NORMAL, bool WITH_DENOISE>
struct ImageBufferTemplate
    : img_buf_impl::ValueBuffer  <WITH_VALUE>,
      img_buf_impl::WeightBuffer <WITH_WEIGHT>,
      img_buf_impl::AlbedoBuffer <WITH_ALBEDO>,
      img_buf_impl::NormalBuffer <WITH_NORMAL>,
      img_buf_impl::DenoiseBuffer<WITH_DENOISE>
{
    ImageBufferTemplate() = default;

    ImageBufferTemplate(int width, int height)
    {
        img_buf_impl::ValueBuffer  <WITH_VALUE>  ::init(width, height);
        img_buf_impl::WeightBuffer <WITH_WEIGHT> ::init(width, height);
        img_buf_impl::AlbedoBuffer <WITH_ALBEDO> ::init(width, height);
        img_buf_impl::NormalBuffer <WITH_NORMAL> ::init(width, height);
        img_buf_impl::DenoiseBuffer<WITH_DENOISE>::init(width, height);
    }
};

/**
 * @brief 渲染算法的输出结果
 *
 * assert(image.is_available())
 *
 * 其他成员可以为!is_available()
 */
struct RenderTarget
{
    Image2D<Spectrum> image;
    Image2D<Spectrum> albedo;
    Image2D<Vec3>     normal;
    Image2D<real>     denoise;

    RenderTarget() = default;

    bool is_valid() const noexcept
    {
        if(!image.is_available())
            return false;
        if(albedo.is_available() && (albedo.width() != image.width() || albedo.height() != image.height()))
            return false;
        if(normal.is_available() && (normal.width() != image.width() || normal.height() != image.height()))
            return false;
        if(denoise.is_available() && (denoise.width() != image.width() || denoise.height() != image.height()))
            return false;
        return true;
    }
};

/**
 * @brief 将film filter作用与图像平面上指定区域的辅助设施
 */
class FilmFilterApplier
{
public:

    /**
     * @brief 图像平面上使用特定film filter的区域
     */
    template<typename...TexelTypes>
    class FilmGrid
    {
        Rect2i pixels_;
        std::shared_ptr<const FilmFilter> film_filter_;

        Rect2i sample_pixels_;
        Rect2 sample_pixel_bound_;

        std::tuple<Image2D<TexelTypes>*...> textures_;

        template<int I, typename FirstTexelType>
        void apply_aux(
            int px, int py, real weight, const FirstTexelType &texel) const;

        template<int I, typename FirstTexelType, typename SecondTexelType, typename...OtherTexelTypes>
        void apply_aux(
            int px, int py, real weight,
            const FirstTexelType &first, const SecondTexelType &second, const OtherTexelTypes&...others) const;

    public:

        explicit FilmGrid(
            const Rect2i &pixel_bound,
            std::shared_ptr<const FilmFilter> film_filter,
            Image2D<TexelTypes>&...textures) noexcept;

        /**
         * @brief 添加一个采样点
         *
         * px、py为以像素为单位的采样点位置，texels按图像像素中的通道顺序给出采样点所有通道的值
         */
        void apply(real px, real py, const TexelTypes &...texels) const noexcept;

        /**
         * @brief 给定像素坐标是否位于该图像区域的有效采样范围内
         *
         * 这是一个必要性检查，即该函数返回true未必有效，但返回false一定无效
         */
        bool in_sample_pixel_bound(real px, real py) const noexcept;

        /**
         * @brief 取得该区域的采样像素的下标范围
         */
        const Rect2i &sample_pixels() const noexcept;
    };

    FilmFilterApplier(int width, int height, std::shared_ptr<const FilmFilter> film_filter) noexcept;

    int width() const noexcept;

    int height() const noexcept;

    /**
     * @brief 将图像重建滤波器绑定到给定Image2D的指定像素区域
     *
     * 绑定后得到的区域可能会写入这部分像素的值，但绝不会写入区域以外的像素
     */
    template<typename...TexelTypes>
    FilmGrid<TexelTypes...> bind_to(const Rect2i &pixel_bound, Image2D<TexelTypes> &...textures) const noexcept;

private:

    int width_;
    int height_;

    std::shared_ptr<const FilmFilter> film_filter_;
};

template<typename...TexelTypes>
template<int I, typename T1>
void FilmFilterApplier::FilmGrid<TexelTypes...>::apply_aux(int px, int py, real weight, const T1 &texel) const
{
    std::get<I>(textures_)->at(py, px) += weight * texel;
}

template<typename...TexelTypes>
template<int I, typename T1, typename T2, typename...Ts>
void FilmFilterApplier::FilmGrid<TexelTypes...>::apply_aux(
    int px, int py, real weight, const T1 &first, const T2 &second, const Ts&...others) const
{
    apply_aux<I>(px, py, weight, first);
    apply_aux<I + 1>(px, py, weight, second, others...);
}

template<typename...TexelTypes>
FilmFilterApplier::FilmGrid<TexelTypes...>::FilmGrid(
    const Rect2i &pixel_bound,
    std::shared_ptr<const FilmFilter> film_filter,
    Image2D<TexelTypes>&...textures) noexcept
    : pixels_(pixel_bound), film_filter_(std::move(film_filter)), textures_{ &textures... }
{
    const real radius = film_filter_->radius();

    sample_pixels_.low.x = static_cast<int>(std::floor(pixel_bound.low.x + real(0.5) - radius));
    sample_pixels_.low.y = static_cast<int>(std::floor(pixel_bound.low.y + real(0.5) - radius));
    sample_pixels_.high.x = static_cast<int>(std::floor(pixel_bound.high.x + real(0.5) + radius));
    sample_pixels_.high.y = static_cast<int>(std::floor(pixel_bound.high.y + real(0.5) + radius));

    sample_pixel_bound_.low.x = static_cast<real>(sample_pixels_.low.x);
    sample_pixel_bound_.low.y = static_cast<real>(sample_pixels_.low.y);
    sample_pixel_bound_.high.x = static_cast<real>(sample_pixels_.high.x) + 1;
    sample_pixel_bound_.high.y = static_cast<real>(sample_pixels_.high.y) + 1;
}

template<typename...TexelTypes>
void FilmFilterApplier::FilmGrid<TexelTypes...>::apply(real px, real py, const TexelTypes&...texels) const noexcept
{
    const real radius = film_filter_->radius();

    const int x_min = (std::max)(pixels_.low.x, static_cast<int>(std::ceil(px - radius - real(0.5))));
    const int y_min = (std::max)(pixels_.low.y, static_cast<int>(std::ceil(py - radius - real(0.5))));
    const int x_max = (std::min)(pixels_.high.x, static_cast<int>(std::floor(px + radius - real(0.5))));
    const int y_max = (std::min)(pixels_.high.y, static_cast<int>(std::floor(py + radius - real(0.5))));

    for(int y = y_min; y <= y_max; ++y)
    {
        const real y_pixel_center = y + real(0.5);
        const real y_relative = std::abs(py - y_pixel_center);
        if(y_relative > radius)
            continue;

        for(int x = x_min; x <= x_max; ++x)
        {
            const real x_pixel_center = x + real(0.5);
            const real x_relative = std::abs(px - x_pixel_center);
            if(x_relative > radius)
                continue;

            const real weight = film_filter_->eval(x_relative, y_relative);
            apply_aux<0>(x, y, weight, texels...);
        }
    }
}

template<typename...TexelTypes>
bool FilmFilterApplier::FilmGrid<TexelTypes...>::in_sample_pixel_bound(real px, real py) const noexcept
{
    return sample_pixel_bound_.low.x <= px && px <= sample_pixel_bound_.high.x &&
           sample_pixel_bound_.low.y <= py && py <= sample_pixel_bound_.high.y;
}

template<typename...TexelTypes>
const Rect2i &FilmFilterApplier::FilmGrid<TexelTypes...>::sample_pixels() const noexcept
{
    return sample_pixels_;
}

inline FilmFilterApplier::FilmFilterApplier(int width, int height, std::shared_ptr<const FilmFilter> film_filter) noexcept
    : width_(width), height_(height), film_filter_(std::move(film_filter))
{
    
}

inline int FilmFilterApplier::width() const noexcept
{
    return width_;
}

inline int FilmFilterApplier::height() const noexcept
{
    return height_;
}

template<typename...TexelTypes>
FilmFilterApplier::FilmGrid<TexelTypes...> FilmFilterApplier::bind_to(const Rect2i &pixel_bound, Image2D<TexelTypes>&...textures) const noexcept
{
    return FilmGrid<TexelTypes...>(pixel_bound, film_filter_, textures...);
}

AGZ_TRACER_END
