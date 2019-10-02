#include <agz/tracer/core/film_filter.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class BoxFilter : public FilmFilter
{
    real radius_ = 0;

public:

    void initialize(real radius)
    {
        AGZ_HIERARCHY_TRY

        radius_ = radius;
        if(radius_ <= 0)
            throw ObjectConstructionException("invalid radius");

        AGZ_HIERARCHY_WRAP("in initializing box filter")
    }

    real radius() const noexcept override
    {
        return radius_;
    }

    real eval(real x, real y) const noexcept override
    {
        return 1;
    }
};

std::shared_ptr<FilmFilter> create_box_filter(
    real radius)
{
    auto ret = std::make_shared<BoxFilter>();
    ret->initialize(radius);
    return ret;
}

AGZ_TRACER_END
