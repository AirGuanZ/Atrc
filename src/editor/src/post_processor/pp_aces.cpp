#include <QGridLayout>
#include <QLabel>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/post_processor/pp_aces.h>

AGZ_EDITOR_BEGIN

ACESWidget::ACESWidget(const PostProcessorWidgetCreator *creator)
    : PostProcessorWidget(creator)
{
    exposure_ = new RealInput(this);
    exposure_->set_value(10);

    auto layout = new QGridLayout(this);
    layout->addWidget(new QLabel("Exposure"), 0, 0);
    layout->addWidget(exposure_, 0, 1);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop);
}

RC<tracer::ConfigGroup> ACESWidget::to_config() const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "aces");
    grp->insert_real("exposure", exposure_->get_value());
    return grp;
}

void ACESWidget::save_asset(AssetSaver &saver) const
{
    saver.write(exposure_->get_value());
}

void ACESWidget::load_asset(AssetLoader &loader)
{
    exposure_->set_value(loader.read<real>());
}

PostProcessorWidget *ACESWidgetCreator::create() const
{
    return new ACESWidget(this);
}

AGZ_EDITOR_END
