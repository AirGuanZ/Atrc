#pragma once

#ifdef USE_OIDN

#include <QCheckBox>

#include <agz/editor/post_processor/post_processor.h>

AGZ_EDITOR_BEGIN

class OIDNWidget : public PostProcessorWidget
{
public:

    explicit OIDNWidget(const PostProcessorWidgetCreator *creator);

    RC<tracer::ConfigGroup> to_config() const override;

    void save_asset(AssetSaver &saver) const override;

    void load_asset(AssetLoader &loader) override;

private:

    QCheckBox *denoise_ldr_ = nullptr;
};

class OIDNWidgetCreator : public PostProcessorWidgetCreator
{
public:

    QString get_type() const override
    {
        return "Denoise";
    }

    PostProcessorWidget *create() const override;
};

AGZ_EDITOR_END

#endif // #ifdef USE_OIDN
