#pragma once

#include <agz/editor/film_filter/film_filter.h>
#include <agz/editor/ui/utility/vec_input.h>

AGZ_EDITOR_BEGIN

class BoxWidget : public FilmFilterWidget
{
public:

    explicit BoxWidget(const FilmFilterWidgetCreator *creator);

    RC<tracer::ConfigGroup> to_config() const override;

    void save_asset(AssetSaver &saver) const override;

    void load_asset(AssetLoader &loader) override;

private:

    RealInput *radius_ = nullptr;
};

class BoxWidgetCreator : public FilmFilterWidgetCreator
{
public:

    QString get_type() const override
    {
        return "Box";
    }

    FilmFilterWidget *create() const override;
};

AGZ_EDITOR_END
