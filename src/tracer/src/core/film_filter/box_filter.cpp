#include <agz/tracer/core/film_filter.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class BoxFilter : public FilmFilter
{
    real radius_ = 0;
    real value_  = 1;

public:

    explicit BoxFilter(real radius)
    {
        AGZ_HIERARCHY_TRY

        radius_ = radius - EPS();
        if(radius_ <= 0)
            throw ObjectConstructionException("invalid radius");
        value_ = 1 / (4 * radius_ * radius_);

        AGZ_HIERARCHY_WRAP("in initializing box filter")
    }

    real radius() const noexcept override
    {
        return radius_;
    }

    real eval(real x, real y) const noexcept override
    {
        return value_;
    }
};

RC<FilmFilter> create_box_filter(
    real radius)
{
    return newRC<BoxFilter>(radius);
}

AGZ_TRACER_END
