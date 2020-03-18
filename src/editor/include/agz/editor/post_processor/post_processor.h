#pragma once

#include <QWidget>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class AssetLoader;
class AssetSaver;

class PostProcessorWidgetCreator;

class PostProcessorWidget : public QWidget
{
    const PostProcessorWidgetCreator *creator_;

public:

    explicit PostProcessorWidget(const PostProcessorWidgetCreator *creator);

    virtual ~PostProcessorWidget() = default;

    QString get_type() const;

    virtual RC<tracer::ConfigGroup> to_config() const = 0;

    virtual void save_asset(AssetSaver &saver) const = 0;

    virtual void load_asset(AssetLoader &loader) = 0;

    virtual RC<tracer::PostProcessor> create_post_processor() const = 0;
};

class PostProcessorWidgetCreator
{
public:

    virtual ~PostProcessorWidgetCreator() = default;

    virtual PostProcessorWidget *create() const = 0;

    virtual QString get_type() const = 0;
};

inline PostProcessorWidget::PostProcessorWidget(const PostProcessorWidgetCreator *creator)
    : creator_(creator)
{
    
}

inline QString PostProcessorWidget::get_type() const
{
    return creator_->get_type();
}

AGZ_EDITOR_END
