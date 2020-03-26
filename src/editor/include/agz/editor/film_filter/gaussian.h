#pragma once

#include <agz/editor/film_filter/film_filter.h>
#include <agz/editor/ui/utility/vec_input.h>

AGZ_EDITOR_BEGIN

class GaussianWidget : public FilmFilterWidget
{
public:

    explicit GaussianWidget(const FilmFilterWidgetCreator *creator);

    RC<tracer::ConfigGroup> to_config() const override;

    void save_asset(AssetSaver &saver) const override;

    void load_asset(AssetLoader &loader) override;

private:

    RealInput *radius_ = nullptr;
    RealInput *alpha_  = nullptr;
};

class GaussianWidgetCreator : public FilmFilterWidgetCreator
{
public:

    QString get_type() const override
    {
        return "Gaussian";
    }

    FilmFilterWidget *create() const override;
};

AGZ_EDITOR_END
