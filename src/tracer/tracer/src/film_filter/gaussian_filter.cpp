#include <agz/tracer/core/film_filter.h>

AGZ_TRACER_BEGIN

class GaussianFilter : public FilmFilter
{
    real radius_;
    real alpha_;
    real exp_;

public:

    using FilmFilter::FilmFilter;

    static std::string description()
    {
        return R"___(
gaussian [FilmFilter]
    radius [real] filter radius
    alpha  [real] gaussian parameter alpha
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext&) override;

    real radius() const noexcept override;

    real eval(real x, real y) const noexcept override;
};

AGZT_IMPLEMENTATION(FilmFilter, GaussianFilter, "gaussian")

void GaussianFilter::initialize(const Config &params, obj::ObjectInitContext&)
{
    AGZ_HIERARCHY_TRY

    real radius = params.child_real("radius");
    if(radius <= 0)
        throw ObjectConstructionException("invalid radius value: " + std::to_string(radius));

    real alpha = params.child_real("alpha");
    if(alpha <= 0)
        throw ObjectConstructionException("invalid alpha value: " + std::to_string(alpha));

    radius_ = radius;
    alpha_  = alpha;
    exp_    = std::exp(-alpha * radius * radius);

    AGZ_HIERARCHY_WRAP("in initializing box filter")
}

real GaussianFilter::radius() const noexcept
{
    return radius_;
}

real GaussianFilter::eval(real rel_x, real rel_y) const noexcept
{
    static auto gaussian = [](real d, real expv, real alpha)
    {
        return (std::max)(real(0), real(std::exp(-alpha * d * d) - expv));
    };
    return gaussian(rel_x, exp_, alpha_) * gaussian(rel_y, exp_, alpha_);
}

AGZ_TRACER_END
