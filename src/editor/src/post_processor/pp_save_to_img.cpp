#include <QGridLayout>
#include <QLabel>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/post_processor/pp_save_to_img.h>
#include <agz/editor/ui/utility/collapsible.h>

AGZ_EDITOR_BEGIN

SaveToImageWidget::SaveToImageWidget(const PostProcessorWidgetCreator *creator)
    : PostProcessorWidget(creator)
{
    use_gamma_corr_ = new QCheckBox("Gamma Correction", this);
    use_gamma_corr_->setChecked(true);

    inv_gamma_ = new RealInput(this);
    inv_gamma_->set_value(real(2.2));

    save_filename_ = new SaveFilenameWidget;
    save_filename_->set_browse_filter("Image (*.png *.jpg *.hdr)");
    save_filename_->set_relative_filename("output.png");

    open_after_saved_ = new QCheckBox("Open After Saved", this);
    open_after_saved_->setChecked(false);

    auto layout = new QGridLayout(this);
    int row = -1;

    layout->addWidget(use_gamma_corr_, ++row, 0, 1, 1);
    layout->addWidget(inv_gamma_,        row, 1, 1, 1);

    layout->addWidget(save_filename_, ++row, 0, 1, 2);

    layout->addWidget(open_after_saved_, ++row, 0, 1, 2);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop);
}

RC<tracer::ConfigGroup> SaveToImageWidget::to_config() const
{
    auto grp = newRC<tracer::ConfigGroup>();

    grp->insert_str("type", "save_to_img");

    if(use_gamma_corr_->isChecked())
        grp->insert_real("inv_gamma", inv_gamma_->get_value());

    grp->insert_str(
        "filename", save_filename_->get_filename(
                        "${scene-directory}/").toStdString());

    grp->insert_bool("open", open_after_saved_->isChecked());

    return grp;
}

void SaveToImageWidget::save_asset(AssetSaver &saver) const
{
    saver.write(int32_t(use_gamma_corr_->isChecked() ? 1 : 0));
    saver.write(inv_gamma_->get_value());

    save_filename_->save_asset(saver);

    saver.write(int32_t(open_after_saved_->isChecked() ? 1 : 0));
}

void SaveToImageWidget::load_asset(AssetLoader &loader)
{
    use_gamma_corr_->setChecked(loader.read<int32_t>() != 0);
    inv_gamma_->set_value(loader.read<real>());

    save_filename_->load_asset(loader);

    open_after_saved_->setChecked(loader.read<int32_t>() != 0);
}

PostProcessorWidget *SaveToImageWidgetCreator::create() const
{
    return new SaveToImageWidget(this);
}

AGZ_EDITOR_END
