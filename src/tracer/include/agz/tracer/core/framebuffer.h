#pragma once

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

/**
 * @brief compute sampling bound of given rect on image
 *
 * image rect: [rect.low.x, rect.high.x] * [rect.low.y * rect.high.y]
 *
 * returned pixel range: [low.x, high.x] * [low.y, high.y]
 */
Rect2i sample_bound_of(real filter_radius, const Rect2i &rect) noexcept;

/**
 * @brief apply image filter to a given sample point
 *
 * pixel range: [rect.low.x, rect.high.x] * [rect.low.y * rect.high.y]
 *
 * for each pixel (int x, int y) with center position (real px, real py):
 *     if |px - sample.x| < r && |py - sample.y| < r:
 *         call func(x, y, |px - sample.x|, |py - sample.y|)
 */
template<typename Func>
void apply_image_filter(
    const Rect2i &pixel_range, real filter_radius,
    const Vec2 &sample, const Func &func);

/**
 * @brief initialize all images with size (w, h) and default pixel value
 */
template<typename...Texels>
void initialize_framebuffers(int w, int h, Image2D<Texels>&...images);

inline Rect2i sample_bound_of(real filter_radius, const Rect2i &rect) noexcept
{
    const int lxi = static_cast<int>(
        std::floor(rect.low.x + real(0.5) - filter_radius));
    const int lyi = static_cast<int>(
        std::floor(rect.low.y + real(0.5) - filter_radius));
    const int hxi = static_cast<int>(
        std::floor(rect.high.x + real(0.49999) + filter_radius));
    const int hyi = static_cast<int>(
        std::floor(rect.high.y + real(0.49999) + filter_radius));
    return { { lxi, lyi }, { hxi, hyi } };
}

template<typename Func>
void apply_image_filter(
    const Rect2i &pixel_range, real filter_radius,
    const Vec2 &sample, const Func &func)
{
    const int x_min = (std::max)(pixel_range.low.x,
        static_cast<int>(std::ceil(sample.x - filter_radius - real(0.5))));
    const int y_min = (std::max)(pixel_range.low.y,
        static_cast<int>(std::ceil(sample.y - filter_radius - real(0.5))));
    const int x_max = (std::min)(pixel_range.high.x,
        static_cast<int>(std::floor(sample.x + filter_radius - real(0.5))));
    const int y_max = (std::min)(pixel_range.high.y,
        static_cast<int>(std::floor(sample.y + filter_radius - real(0.5))));

    real y_rel = std::abs(y_min + real(0.5) - sample.y);
    for(int y = y_min; y <= y_max; ++y)
    {
        AGZ_SCOPE_GUARD({ y_rel += 1; });
        if(y_rel > filter_radius)
            continue;

        real x_rel = std::abs(x_min + real(0.5) - sample.x);
        for(int x = x_min; x <= x_max; ++x)
        {
            AGZ_SCOPE_GUARD({ x_rel += 1; });
            if(x_rel > filter_radius)
                continue;

            func(x, y, x_rel, y_rel);
        }
    }
}

template<typename ... Texels>
void initialize_framebuffers(int w, int h, Image2D<Texels>&...images)
{
    (images.initialize(h, w), ...);
}

AGZ_TRACER_END
