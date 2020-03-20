#pragma once

#include <agz/editor/post_processor/post_processor.h>
#include <agz/editor/ui/utility/vec_input.h>

AGZ_EDITOR_BEGIN

class ACESWidget : public PostProcessorWidget
{
public:

    explicit ACESWidget(const PostProcessorWidgetCreator *creator);

    RC<tracer::ConfigGroup> to_config() const override;

    void save_asset(AssetSaver &saver) const override;

    void load_asset(AssetLoader &loader) override;

private:

    RealInput *exposure_ = nullptr;
};

class ACESWidgetCreator : public PostProcessorWidgetCreator
{
public:

    QString get_type() const override
    {
        return "ACES Tone Mapping";
    }

    PostProcessorWidget *create() const override;
};

AGZ_EDITOR_END
