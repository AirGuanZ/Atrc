#include <QGridLayout>
#include <QLabel>

#include <agz/editor/film_filter/box.h>
#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>

AGZ_EDITOR_BEGIN

BoxWidget::BoxWidget(const FilmFilterWidgetCreator *creator)
    : FilmFilterWidget(creator)
{
    radius_ = new RealInput(this);
    radius_->set_value(real(0.5));

    auto layout = new QGridLayout(this);

    layout->addWidget(new QLabel("Radius"), 0, 0);
    layout->addWidget(radius_, 0, 1);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
}

RC<tracer::ConfigGroup> BoxWidget::to_config() const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "box");
    grp->insert_real("radius", radius_->get_value());
    return grp;
}

void BoxWidget::save_asset(AssetSaver &saver) const
{
    saver.write(radius_->get_value());
}

void BoxWidget::load_asset(AssetLoader &loader)
{
    radius_->set_value(loader.read<real>());
}

FilmFilterWidget *BoxWidgetCreator::create() const
{
    return new BoxWidget(this);
}

AGZ_EDITOR_END
