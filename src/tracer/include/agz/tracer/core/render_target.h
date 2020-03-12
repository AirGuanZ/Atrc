#pragma once

#include <agz/tracer/core/framebuffer.h>
#include <agz/tracer/core/film_filter.h>
#include <agz/utility/texture.h>

AGZ_TRACER_BEGIN

namespace img_buf_impl
{
    template<bool WITH_VALUE> struct ValueBuffer { void init(int w, int h) { } };
    template<> struct ValueBuffer<true>
    { Image2D<Spectrum> value; void init(int w, int h){ value.initialize(h, w); } };

    template<bool WITH_WEIGHT> struct WeightBuffer { void init(int w, int h) { } };
    template<> struct WeightBuffer<true>
    { Image2D<real> weight; void init(int w, int h) { weight.initialize(h, w); } };

    template<bool WITH_ALBEDO> struct AlbedoBuffer { void init(int w, int h) { } };
    template<> struct AlbedoBuffer<true>
    { Image2D<Spectrum> albedo; void init(int w, int h) { albedo.initialize(h, w); } };

    template<bool WITH_NORMAL> struct NormalBuffer { void init(int w, int h) { } };
    template<> struct NormalBuffer<true>
    { Image2D<Vec3> normal; void init(int w, int h) { normal.initialize(h, w); } };

    template<bool WITH_DENOISE> struct DenoiseBuffer { void init(int w, int h) { } };
    template<> struct DenoiseBuffer<true>
    { Image2D<real> denoise; void init(int w, int h) { denoise.initialize(h, w); } };
}

/**
 * @brief standard template for storing intermediate rendering results
 *
 * use template arguments to select between following members:
 * Image2D<Spectrum> value
 * Image2D<real>     weight
 * Image2D<Spectrum> albedo
 * Image2D<Vec3>     normal
 * Image2D<real>     denoise
 */
template<bool WITH_VALUE,
         bool WITH_WEIGHT,
         bool WITH_ALBEDO,
         bool WITH_NORMAL,
         bool WITH_DENOISE>
struct ImageBufferTemplate
    : img_buf_impl::ValueBuffer  <WITH_VALUE>,
      img_buf_impl::WeightBuffer <WITH_WEIGHT>,
      img_buf_impl::AlbedoBuffer <WITH_ALBEDO>,
      img_buf_impl::NormalBuffer <WITH_NORMAL>,
      img_buf_impl::DenoiseBuffer<WITH_DENOISE>
{
    ImageBufferTemplate() = default;

    ImageBufferTemplate(int width, int height);
};

/**
 * @brief output of rendering algorithm
 *
 * assert(image.is_available())
 */
struct RenderTarget
{
    Image2D<Spectrum> image;
    Image2D<Spectrum> albedo;
    Image2D<Vec3>     normal;
    Image2D<real>     denoise;

    RenderTarget() = default;

    bool is_valid() const noexcept;
};

/**
 * @brief helper class for reconstructing the image buffer
 *  with given filter function and sample points
 */
class FilmFilterApplier
{
public:

    /**
     * @brief copy of a rect region on film
     */
    template<typename...TexelTypes>
    class FilmGrid
    {
        Rect2i pixel_range_;
        Vec2i grid_size_;
        RC<const FilmFilter> film_filter_;

        Rect2i sample_pixels_;
        Rect2 sample_pixel_bound_;

        std::tuple<Image2D<TexelTypes>...> grids_;

        template<int I, typename T1>
        void resize_grid(int width, int height);

        template<int I, typename T1, typename T2, typename...Ts>
        void resize_grid(int width, int height);

        template<int I, typename T1>
        void apply_aux(int px, int py, real weight, const T1 &texel);

        template<int I, typename T1, typename T2, typename...Ts>
        void apply_aux(
            int px, int py, real weight, const T1 &t1, const T2 &t2,
            const Ts&...ts);

        template<int I, typename T1>
        void merge_into_aux(Image2D<T1> &texture) const;

        template<int I, typename T1, typename T2, typename...Ts>
        void merge_into_aux(
            Image2D<T1> &t1, Image2D<T2> &t2, Image2D<Ts> &...ts) const;

        template<int I, typename T1>
        void clear_aux(const T1 &t1);

        template<int I, typename T1, typename T2, typename...Ts>
        void clear_aux(const T1 &t1, const T2 &t2, const Ts &...ts);

    public:

        FilmGrid(
            const Rect2i &pixel_range,
            RC<const FilmFilter> film_filter);

        void set_pixel_range(const Rect2i &pixel_range);

        /**
         * @brief add a sample point
         */
        void apply(real px, real py, const TexelTypes&...texels) noexcept;

        /**
         * @brief is the given pixel coordinate in non-zero sample bounds
         */
        bool in_sample_pixel_bound(real px, real py);

        /**
         * @brief get sample pixel range
         */
        const Rect2i &sample_pixels() const noexcept;

        /**
         * @brief add grid data to full image buffer
         */
        void merge_into(Image2D<TexelTypes>&...textures) const;

        /**
         * @brief clear the grid data
         */
        void clear(const TexelTypes&...texels);
    };

    /**
     * @brief ref to a rect region on film
     */
    template<typename...TexelTypes>
    class FilmGridView
    {
        Rect2i pixels_;
        RC<const FilmFilter> film_filter_;

        Rect2i sample_pixels_;
        Rect2 sample_pixel_bound_;

        std::tuple<Image2D<TexelTypes>*...> textures_;

        template<int I, typename FirstTexelType>
        void apply_aux(
            int px, int py, real weight, const FirstTexelType &texel) const;

        template<int I, typename FirstTexelType, typename SecondTexelType,
                                                 typename...OtherTexelTypes>
        void apply_aux(
            int px, int py, real weight,
            const FirstTexelType &first, const SecondTexelType &second,
            const OtherTexelTypes&...others) const;

    public:

        FilmGridView(
            const Rect2i &pixel_bound,
            RC<const FilmFilter> film_filter,
            Image2D<TexelTypes>&...textures) noexcept;

        /**
         * @brief add a sample point
         */
        void apply(real px, real py, const TexelTypes &...texels) const noexcept;

        /**
         * @brief is the given pixel coordinate in non-zero sample bounds
         */
        bool in_sample_pixel_bound(real px, real py) const noexcept;

        /**
         * @brief get sample pixel range
         */
        const Rect2i &sample_pixels() const noexcept;
    };

    FilmFilterApplier(
        int width, int height, RC<const FilmFilter> film_filter) noexcept;

    int width() const noexcept;

    int height() const noexcept;

    /**
     * @brief create subgrid bound to a pixel range on given textures
     *
     * values in the pixel range can be written by the returned subgrid
     */
    template<typename...TexelTypes>
    FilmGridView<TexelTypes...> create_subgrid_view(
        const Rect2i &pixel_bound,
        Image2D<TexelTypes> &...textures) const noexcept;

    /**
     * @brief create subgrid representing the given pixel range
     */
    template<typename...TexelTypes>
    FilmGrid<TexelTypes...> create_subgrid(
        const Rect2i &pixel_bound) const noexcept;

private:

    int width_;
    int height_;

    RC<const FilmFilter> film_filter_;
};

template<bool WITH_VALUE,
         bool WITH_WEIGHT,
         bool WITH_ALBEDO,
         bool WITH_NORMAL,
         bool WITH_DENOISE>
ImageBufferTemplate<
    WITH_VALUE, WITH_WEIGHT, WITH_ALBEDO, WITH_NORMAL, WITH_DENOISE>
    ::ImageBufferTemplate(int width, int height)
{
    img_buf_impl::ValueBuffer  <WITH_VALUE>  ::init(width, height);
    img_buf_impl::WeightBuffer <WITH_WEIGHT> ::init(width, height);
    img_buf_impl::AlbedoBuffer <WITH_ALBEDO> ::init(width, height);
    img_buf_impl::NormalBuffer <WITH_NORMAL> ::init(width, height);
    img_buf_impl::DenoiseBuffer<WITH_DENOISE>::init(width, height);
}

inline bool RenderTarget::is_valid() const noexcept
{
    if(!image.is_available())
        return false;
    if(albedo.is_available() && albedo.size() != image.size())
        return false;
    if(normal.is_available() && normal.size() != image.size())
        return false;
    if(denoise.is_available() && denoise.size() != image.size())
        return false;
    return true;
}

template<typename...TexelTypes>
template<int I, typename T1>
void FilmFilterApplier::FilmGrid<TexelTypes...>::resize_grid(
    int width, int height)
{
    auto &tex = std::get<I>(grids_);
    if(tex.width() < width || tex.height() < height)
        tex.initialize(height, width);
    else
        tex.clear(T1());
}

template<typename...TexelTypes>
template<int I, typename T1, typename T2, typename...Ts>
void FilmFilterApplier::FilmGrid<TexelTypes...>::resize_grid(
    int width, int height)
{
    resize_grid<I, T1>(width, height);
    resize_grid<I + 1, T2, Ts...>(width, height);
}

template<typename...TexelTypes>
template<int I, typename T1>
void FilmFilterApplier::FilmGrid<TexelTypes...>::apply_aux(
    int px, int py, real weight, const T1 &texel)
{
    std::get<I>(grids_).at(py, px) += weight * texel;
}

template<typename...TexelTypes>
template<int I, typename T1, typename T2, typename...Ts>
void FilmFilterApplier::FilmGrid<TexelTypes...>::apply_aux(
    int px, int py, real weight, const T1 &t1, const T2 &t2, const Ts &... ts)
{
    apply_aux<I>(px, py, weight, t1);
    apply_aux<I + 1>(px, py, weight, t2, ts...);
}

template<typename...TexelTypes>
template<int I, typename T1>
void FilmFilterApplier::FilmGrid<TexelTypes...>::merge_into_aux(
    Image2D<T1> &texture) const
{
    auto &local_tex = std::get<I>(grids_);
    for(int y = pixel_range_.low.y, local_y = 0;
        y <= pixel_range_.high.y; ++y, ++local_y)
    {
        for(int x = pixel_range_.low.x, local_x = 0;
            x <= pixel_range_.high.x; ++x, ++local_x)
            texture.at(y, x) = local_tex(local_y, local_x);
    }
}

template<typename...TexelTypes>
template<int I, typename T1, typename T2, typename ... Ts>
void FilmFilterApplier::FilmGrid<TexelTypes...>::merge_into_aux(
    Image2D<T1> &t1, Image2D<T2> &t2, Image2D<Ts> &...ts) const
{
    merge_into_aux<I>(t1);
    merge_into_aux<I + 1>(t2, ts...);
}

template<typename...TexelTypes>
template<int I, typename T1>
void FilmFilterApplier::FilmGrid<TexelTypes...>::clear_aux(const T1 &t1)
{
    std::get<I>(grids_).clear(t1);
}

template<typename...TexelTypes>
template<int I, typename T1, typename T2, typename ... Ts>
void FilmFilterApplier::FilmGrid<TexelTypes...>::clear_aux(
    const T1 &t1, const T2 &t2, const Ts&...ts)
{
    clear_aux<I>(t1);
    clear_aux<I + 1>(t2, ts...);
}

template<typename...TexelTypes>
FilmFilterApplier::FilmGrid<TexelTypes...>::FilmGrid(
    const Rect2i &pixel_range, RC<const FilmFilter> film_filter)
    : film_filter_(std::move(film_filter))
{
    set_pixel_range(pixel_range);
}

template<typename...TexelTypes>
void FilmFilterApplier::FilmGrid<TexelTypes...>::set_pixel_range(
    const Rect2i &pixel_range)
{
    pixel_range_ = pixel_range;
    grid_size_.x = pixel_range_.high.x - pixel_range_.low.x + 1;
    grid_size_.y = pixel_range_.high.y - pixel_range_.low.y + 1;

    resize_grid<0, TexelTypes...>(grid_size_.x, grid_size_.y);

    const real radius = film_filter_->radius();

    sample_pixels_ = sample_bound_of(radius, pixel_range);

    sample_pixel_bound_.low.x  = static_cast<real>(sample_pixels_.low.x);
    sample_pixel_bound_.low.y  = static_cast<real>(sample_pixels_.low.y);
    sample_pixel_bound_.high.x = static_cast<real>(sample_pixels_.high.x) + 1;
    sample_pixel_bound_.high.y = static_cast<real>(sample_pixels_.high.y) + 1;
}

template<typename...TexelTypes>
void FilmFilterApplier::FilmGrid<TexelTypes...>::apply(
    real px, real py, const TexelTypes&...texels) noexcept
{
    apply_image_filter(
        pixel_range_, film_filter_->radius(), { px, py },
        [&](int pix, int piy, real rel_x, real rel_y)
    {
        const real weight = film_filter_->eval(rel_x, rel_y);
        apply_aux<0>(pix - pixel_range_.low.x,
                     piy - pixel_range_.low.y, weight, texels...);
    });
}

template<typename...TexelTypes>
bool FilmFilterApplier::FilmGrid<TexelTypes...>::in_sample_pixel_bound(
    real px, real py)
{
    return sample_pixel_bound_.low.x <= px && px <= sample_pixel_bound_.high.x &&
           sample_pixel_bound_.low.y <= py && py <= sample_pixel_bound_.high.y;
}

template<typename...TexelTypes>
const Rect2i &FilmFilterApplier::FilmGrid<TexelTypes...>
    ::sample_pixels() const noexcept
{
    return sample_pixels_;
}

template<typename...TexelTypes>
void FilmFilterApplier::FilmGrid<TexelTypes...>::clear(const TexelTypes&...texels)
{
    clear_aux<0>(texels...);
}

template<typename...TexelTypes>
void FilmFilterApplier::FilmGrid<TexelTypes...>::merge_into(
    Image2D<TexelTypes> &...textures) const
{
    merge_into_aux<0>(textures...);
}

template<typename...TexelTypes>
template<int I, typename T1>
void FilmFilterApplier::FilmGridView<TexelTypes...>::apply_aux(
    int px, int py, real weight, const T1 &texel) const
{
    std::get<I>(textures_)->at(py, px) += weight * texel;
}

template<typename...TexelTypes>
template<int I, typename T1, typename T2, typename...Ts>
void FilmFilterApplier::FilmGridView<TexelTypes...>::apply_aux(
    int px, int py, real weight, const T1 &first, const T2 &second,
    const Ts&...others) const
{
    apply_aux<I>(px, py, weight, first);
    apply_aux<I + 1>(px, py, weight, second, others...);
}

template<typename...TexelTypes>
FilmFilterApplier::FilmGridView<TexelTypes...>::FilmGridView(
    const Rect2i &pixel_bound,
    RC<const FilmFilter> film_filter,
    Image2D<TexelTypes>&...textures) noexcept
    : pixels_(pixel_bound),
      film_filter_(std::move(film_filter)),
      textures_{ &textures... }
{
    const real radius = film_filter_->radius();

    sample_pixels_ = sample_bound_of(radius, pixel_bound);

    sample_pixel_bound_.low.x = static_cast<real>(sample_pixels_.low.x);
    sample_pixel_bound_.low.y = static_cast<real>(sample_pixels_.low.y);
    sample_pixel_bound_.high.x = static_cast<real>(sample_pixels_.high.x) + 1;
    sample_pixel_bound_.high.y = static_cast<real>(sample_pixels_.high.y) + 1;
}

template<typename...TexelTypes>
void FilmFilterApplier::FilmGridView<TexelTypes...>::apply(
    real px, real py, const TexelTypes&...texels) const noexcept
{
    apply_image_filter(
        pixels_, film_filter_->radius(), { px, py },
        [&](int pix, int piy, real rel_x, real rel_y)
    {
        const real weight = film_filter_->eval(rel_x, rel_y);
        apply_aux<0>(pix, piy, weight, texels...);
    });
}

template<typename...TexelTypes>
bool FilmFilterApplier::FilmGridView<TexelTypes...>::in_sample_pixel_bound(
    real px, real py) const noexcept
{
    return sample_pixel_bound_.low.x <= px && px <= sample_pixel_bound_.high.x &&
           sample_pixel_bound_.low.y <= py && py <= sample_pixel_bound_.high.y;
}

template<typename...TexelTypes>
const Rect2i &FilmFilterApplier::FilmGridView<TexelTypes...>
    ::sample_pixels() const noexcept
{
    return sample_pixels_;
}

inline FilmFilterApplier::FilmFilterApplier(
    int width, int height, RC<const FilmFilter> film_filter) noexcept
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
FilmFilterApplier::FilmGridView<TexelTypes...> FilmFilterApplier
    ::create_subgrid_view(
    const Rect2i &pixel_bound, Image2D<TexelTypes>&...textures) const noexcept
{
    return FilmGridView<TexelTypes...>(pixel_bound, film_filter_, textures...);
}

template<typename...TexelTypes>
FilmFilterApplier::FilmGrid<TexelTypes...> FilmFilterApplier::create_subgrid(
    const Rect2i &pixel_bound) const noexcept
{
    return FilmGrid<TexelTypes...>(pixel_bound, film_filter_);
}

AGZ_TRACER_END
