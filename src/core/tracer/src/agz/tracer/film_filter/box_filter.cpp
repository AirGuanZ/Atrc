#include <agz/tracer/core/film_filter.h>

AGZ_TRACER_BEGIN

class BoxFilter : public FilmFilter
{
    real radius_ = 0;

public:

    explicit BoxFilter(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

    static std::string description()
    {
        return R"___(
box [FilmFilter]
    radius [real] side length of filter box / 2
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext&) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        real radius = params.child_real("radius");
        
        initialize(radius);

        AGZ_HIERARCHY_WRAP("in initializing box filter")
    }

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

FilmFilter *create_box_filter(
    real radius,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<BoxFilter>(customed_flag);
    ret->initialize(radius);
    return ret;
}

AGZT_IMPLEMENTATION(FilmFilter, BoxFilter, "box")

AGZ_TRACER_END
