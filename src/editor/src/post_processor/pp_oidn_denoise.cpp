#ifdef USE_OIDN

#include <QVBoxLayout>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/post_processor/pp_oidn_denoise.h>

AGZ_EDITOR_BEGIN

OIDNWidget::OIDNWidget(const PostProcessorWidgetCreator *creator)
    : PostProcessorWidget(creator)
{
    denoise_ldr_ = new QCheckBox("Denoise in LDR", this);
    denoise_ldr_->setChecked(false);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(denoise_ldr_);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop);
}

RC<tracer::ConfigGroup> OIDNWidget::to_config() const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "oidn_denoiser");
    grp->insert_bool("clamp", denoise_ldr_->isChecked());
    return grp;
}

void OIDNWidget::save_asset(AssetSaver &saver) const
{
    saver.write(int32_t(denoise_ldr_->isChecked() ? 1 : 0));
}

void OIDNWidget::load_asset(AssetLoader &loader)
{
    denoise_ldr_->setChecked(loader.read<int32_t>() != 0);
}

PostProcessorWidget *OIDNWidgetCreator::create() const
{
    return new OIDNWidget(this);
}

AGZ_EDITOR_END

#endif // #ifdef USE_OIDN