#pragma once

#include <agz/editor/post_processor/post_processor.h>
#include <agz/editor/ui/utility/vec_input.h>

AGZ_EDITOR_BEGIN

class GammaWidget : public PostProcessorWidget
{
public:

    explicit GammaWidget(const PostProcessorWidgetCreator *creator);

    RC<tracer::ConfigGroup> to_config() const override;

    void save_asset(AssetSaver &saver) const override;

    void load_asset(AssetLoader &loader) override;

private:

    RealInput *inv_gamma_ = nullptr;
};

class GammaWidgetCreator : public PostProcessorWidgetCreator
{
public:

    QString get_type() const override
    {
        return "Gamma Correction";
    }

    PostProcessorWidget *create() const override;
};

AGZ_EDITOR_END
