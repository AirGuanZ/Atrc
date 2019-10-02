#include <agz/tracer/factory/creator/film_creators.h>
#include <agz/tracer/factory/raw/film.h>

AGZ_TRACER_FACTORY_BEGIN

namespace film
{
    
    class NativeFilmCreator : public Creator<Film>
    {
    public:

        std::string name() const override
        {
            return "native";
        }

        std::shared_ptr<Film> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            int width = params.child_int("width");
            int height = params.child_int("height");
            return create_native_film(width, height);
        }
    };

    class FilteredFilmCreator : public Creator<Film>
    {
    public:

        std::string name() const override
        {
            return "filtered";
        }

        std::shared_ptr<Film> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            int width = params.child_int("width");
            int height = params.child_int("height");
            auto filter = context.create<FilmFilter>(params.child_group("filter"));
            return create_filtered_film(width, height, std::move(filter));
        }
    };

} // namespace film

void initialize_film_factory(Factory<Film> &factory)
{
    factory.add_creator(std::make_unique<film::NativeFilmCreator>());
    factory.add_creator(std::make_unique<film::FilteredFilmCreator>());
}

AGZ_TRACER_FACTORY_END
