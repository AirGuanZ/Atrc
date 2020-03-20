#pragma once

#include <agz/editor/post_processor/post_processor.h>
#include <agz/editor/ui/save_filename.h>
#include <agz/editor/ui/utility/vec_input.h>

AGZ_EDITOR_BEGIN

class SaveToImageWidget : public PostProcessorWidget
{
public:

    explicit SaveToImageWidget(const PostProcessorWidgetCreator *creator);

    RC<tracer::ConfigGroup> to_config() const override;

    void save_asset(AssetSaver &saver) const override;

    void load_asset(AssetLoader &loader) override;

private:

    QCheckBox *use_gamma_corr_ = nullptr;
    RealInput *inv_gamma_      = nullptr;

    SaveFilenameWidget *save_filename_ = nullptr;

    QCheckBox *open_after_saved_ = nullptr;
};

class SaveToImageWidgetCreator : public PostProcessorWidgetCreator
{
public:

    QString get_type() const override
    {
        return "Save to Image";
    }

    PostProcessorWidget *create() const override;
};

AGZ_EDITOR_END
