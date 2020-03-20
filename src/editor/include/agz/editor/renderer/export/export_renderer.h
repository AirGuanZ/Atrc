#pragma once

#include <QComboBox>
#include <QVBoxLayout>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class AssetLoader;
class AssetSaver;

class ExportRendererWidget : public QWidget
{
public:

    using QWidget::QWidget;

    virtual ~ExportRendererWidget() = default;

    virtual RC<tracer::ConfigGroup> to_config() const = 0;

    virtual void save_asset(AssetSaver &saver) const = 0;

    virtual void load_asset(AssetLoader &loader) = 0;
};

class ExportRendererPanel : public QWidget
{
public:

    ExportRendererPanel();

    RC<tracer::ConfigGroup> to_config() const;

    void save_asset(AssetSaver &saver) const;

    void load_asset(AssetLoader &loader);

private:

    QComboBox            *type_selector_   = nullptr;
    ExportRendererWidget *renderer_widget_ = nullptr;

    QVBoxLayout *layout_ = nullptr;
};

AGZ_EDITOR_END
