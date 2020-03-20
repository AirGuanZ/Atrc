#pragma once

#include <agz/editor/post_processor/post_processor.h>
#include <agz/editor/ui/save_filename.h>

AGZ_EDITOR_BEGIN

class SaveGBufferWidget : public PostProcessorWidget
{
public:

    explicit SaveGBufferWidget(const PostProcessorWidgetCreator *creator);

    RC<tracer::ConfigGroup> to_config() const override;

    void save_asset(AssetSaver &saver) const override;

    void load_asset(AssetLoader &loader) override;

private:

    SaveFilenameWidget *albedo_filename_ = nullptr;
    SaveFilenameWidget *normal_filename_ = nullptr;
};

class SaveGBufferWidgetCreator : public PostProcessorWidgetCreator
{
public:

    QString get_type() const override
    {
        return "Save G-Buffer";
    }

    PostProcessorWidget *create() const override;
};

AGZ_EDITOR_END
