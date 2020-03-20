#include <QVBoxLayout>

#include <agz/editor/post_processor/pp_save_gbuffer.h>
#include <agz/editor/ui/utility/collapsible.h>

AGZ_EDITOR_BEGIN

SaveGBufferWidget::SaveGBufferWidget(const PostProcessorWidgetCreator *creator)
    : PostProcessorWidget(creator)
{
    albedo_filename_ = new SaveFilenameWidget(this);
    normal_filename_ = new SaveFilenameWidget(this);

    albedo_filename_->set_browse_filter("Image (*.png)");
    albedo_filename_->set_relative_filename("albedo.png");

    normal_filename_->set_browse_filter("Image (*.png)");
    normal_filename_->set_relative_filename("normal.png");

    auto albedo_sec = new Collapsible(this, "Albedo Filename");
    auto normal_sec = new Collapsible(this, "Normal Filename");

    albedo_sec->set_content_widget(albedo_filename_);
    normal_sec->set_content_widget(normal_filename_);

    albedo_sec->open();
    normal_sec->open();

    albedo_sec->set_toggle_button_disabled(true);
    normal_sec->set_toggle_button_disabled(true);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(albedo_sec);
    layout->addWidget(normal_sec);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop);
}

RC<tracer::ConfigGroup> SaveGBufferWidget::to_config() const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "save_gbuffer_to_png");
    grp->insert_str(
        "albedo", albedo_filename_->get_filename(
                        "${scene-directory}/").toStdString());
    grp->insert_str(
        "normal", normal_filename_->get_filename(
                        "${scene-directory}/").toStdString());
    return grp;

}

void SaveGBufferWidget::save_asset(AssetSaver &saver) const
{
    albedo_filename_->save_asset(saver);
    normal_filename_->save_asset(saver);
}

void SaveGBufferWidget::load_asset(AssetLoader &loader)
{
    albedo_filename_->load_asset(loader);
    normal_filename_->load_asset(loader);
}

PostProcessorWidget *SaveGBufferWidgetCreator::create() const
{
    return new SaveGBufferWidget(this);
}

AGZ_EDITOR_END
