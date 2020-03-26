#include <QGridLayout>
#include <QLabel>

#include <agz/editor/film_filter/gaussian.h>
#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>

AGZ_EDITOR_BEGIN

GaussianWidget::GaussianWidget(const FilmFilterWidgetCreator *creator)
    : FilmFilterWidget(creator)
{
    radius_ = new RealInput(this);
    radius_->set_value(real(0.8));

    alpha_ = new RealInput(this);
    alpha_->set_value(real(2));

    auto layout = new QGridLayout(this);

    layout->addWidget(new QLabel("Radius"), 0, 0);
    layout->addWidget(radius_, 0, 1);

    layout->addWidget(new QLabel("Alpha"), 1, 0);
    layout->addWidget(alpha_, 1, 1);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
}

RC<tracer::ConfigGroup> GaussianWidget::to_config() const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "gaussian");
    grp->insert_real("radius", radius_->get_value());
    grp->insert_real("alpha", alpha_->get_value());
    return grp;
}

void GaussianWidget::save_asset(AssetSaver &saver) const
{
    saver.write(radius_->get_value());
    saver.write(alpha_->get_value());
}

void GaussianWidget::load_asset(AssetLoader &loader)
{
    radius_->set_value(loader.read<real>());
    alpha_->set_value(loader.read<real>());
}

FilmFilterWidget *GaussianWidgetCreator::create() const
{
    return new GaussianWidget(this);
}

AGZ_EDITOR_END
