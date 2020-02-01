#pragma once

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

/**
 * @brief filter function for reconstructing the image with samples
 * 
 * see http://alvyray.com/Memos/CG/Microsoft/6_pixel.pdf
 */
class FilmFilter
{
public:

    virtual ~FilmFilter() = default;

    /**
     * @brief max non-zero radius in pixels
     */
    virtual real radius() const noexcept = 0;

    /**
     * @brief eval function value
     */
    virtual real eval(real x, real y) const noexcept = 0;
};

AGZ_TRACER_END
