#pragma once

#include <QSpinBox>

#include <agz/editor/post_processor/post_processor.h>

AGZ_EDITOR_BEGIN

class ResizeWidget : public PostProcessorWidget
{
public:

    explicit ResizeWidget(const PostProcessorWidgetCreator *creator);

    RC<tracer::ConfigGroup> to_config() const override;

    void save_asset(AssetSaver &saver) const override;

    void load_asset(AssetLoader &loader) override;

private:

    QSpinBox *width_;
    QSpinBox *height_;
};

class ResizeWidgetCreator : public PostProcessorWidgetCreator
{
public:

    QString get_type() const override
    {
        return "Resize";
    }

    PostProcessorWidget *create() const override;
};

AGZ_EDITOR_END
