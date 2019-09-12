#include <agz/tracer/core/film_filter.h>

AGZ_TRACER_BEGIN

class BoxFilter : public FilmFilter
{
    real radius_ = 0;

public:

    using FilmFilter::FilmFilter;

    static std::string description()
    {
        return R"___(
box [FilmFilter]
    radius [real] side length of filter box / 2
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext&) override;

    real radius() const noexcept override;

    real eval(real x, real y) const noexcept override;
};

AGZT_IMPLEMENTATION(FilmFilter, BoxFilter, "box")

void BoxFilter::initialize(const Config &params, obj::ObjectInitContext&)
{
    AGZ_HIERARCHY_TRY

    init_customed_flag(params);

    radius_ = params.child_real("radius");
    if(radius_ <= 0)
        throw ObjectConstructionException("invalid radius");

    AGZ_HIERARCHY_WRAP("in initializing box filter")
}

real BoxFilter::radius() const noexcept
{
    return radius_;
}

real BoxFilter::eval(real, real) const noexcept
{
    return 1;
}

AGZ_TRACER_END
