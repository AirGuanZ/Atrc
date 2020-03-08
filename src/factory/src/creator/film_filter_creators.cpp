#include <agz/factory/creator/film_filter_creators.h>
#include <agz/tracer/create/film_filter.h>

AGZ_TRACER_FACTORY_BEGIN

namespace film_filter
{
    
    class BoxCreator : public Creator<FilmFilter>
    {
    public:

        std::string name() const override
        {
            return "box";
        }

        std::shared_ptr<FilmFilter> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const real radius = params.child_real("radius");
            return create_box_filter(radius);
        }
    };

    class GaussianCreator : public Creator<FilmFilter>
    {
    public:

        std::string name() const override
        {
            return "gaussian";
        }

        std::shared_ptr<FilmFilter> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const real radius = params.child_real("radius");
            const real alpha = params.child_real("alpha");
            return create_gaussian_filter(radius, alpha);
        }
    };

} // namespace film_filter

void initialize_film_filter_factory(Factory<FilmFilter> &factory)
{
    factory.add_creator(std::make_unique<film_filter::BoxCreator>());
    factory.add_creator(std::make_unique<film_filter::GaussianCreator>());
}

AGZ_TRACER_FACTORY_END
