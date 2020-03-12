#include <agz/tracer/core/film_filter.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class GaussianFilter : public FilmFilter
{
    real radius_ = 0;
    real alpha_  = 0;
    real exp_    = 0;

public:

    GaussianFilter(real radius, real alpha)
    {
        AGZ_HIERARCHY_TRY

        if(radius <= 0)
            throw ObjectConstructionException(
                "invalid radius value: " + std::to_string(radius));
            
        if(alpha <= 0)
            throw ObjectConstructionException(
                "invalid alpha value: " + std::to_string(alpha));

        radius_ = radius;
        alpha_  = alpha;
        exp_    = std::exp(-alpha * radius * radius);

        AGZ_HIERARCHY_WRAP("in initializing box filter")
    }

    real radius() const noexcept override
    {
        return radius_;
    }

    real eval(real rel_x, real rel_y) const noexcept override
    {
        static auto gaussian = [](real d, real expv, real alpha)
        {
            return (std::max)(real(0), real(std::exp(-alpha * d * d) - expv));
        };
        return gaussian(rel_x, exp_, alpha_) * gaussian(rel_y, exp_, alpha_);
    }
};

RC<FilmFilter> create_gaussian_filter(
    real radius, real alpha)
{
    return newRC<GaussianFilter>(radius, alpha);
}

AGZ_TRACER_END
