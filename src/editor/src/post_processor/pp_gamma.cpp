#include <QGridLayout>
#include <QLabel>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/post_processor/pp_gamma.h>

AGZ_EDITOR_BEGIN

GammaWidget::GammaWidget(const PostProcessorWidgetCreator *creator)
    : PostProcessorWidget(creator)
{
    inv_gamma_ = new RealInput(this);
    inv_gamma_->set_value(real(2.2));

    auto layout = new QGridLayout(this);
    layout->addWidget(new QLabel("1 / Gamma"), 0, 0);
    layout->addWidget(inv_gamma_, 0, 1);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop);
}

RC<tracer::ConfigGroup> GammaWidget::to_config() const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "gamma");
    grp->insert_real("inv_gamma", inv_gamma_->get_value());
    return grp;
}

void GammaWidget::save_asset(AssetSaver &saver) const
{
    saver.write(inv_gamma_->get_value());
}

void GammaWidget::load_asset(AssetLoader &loader)
{
    inv_gamma_->set_value(loader.read<real>());
}

PostProcessorWidget *GammaWidgetCreator::create() const
{
    return new GammaWidget(this);
}

AGZ_EDITOR_END
