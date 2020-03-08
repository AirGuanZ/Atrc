#pragma once

#include <QComboBox>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class ExportRendererWidget : public QWidget
{
public:

    using QWidget::QWidget;

    virtual ~ExportRendererWidget() = default;

    virtual std::shared_ptr<tracer::ConfigGroup> to_config() const = 0;
};

class ExportRendererPanel : public QWidget
{
public:

    ExportRendererPanel();

    std::shared_ptr<tracer::ConfigGroup> to_config() const;

private:

    QComboBox            *type_selector_   = nullptr;
    ExportRendererWidget *renderer_widget_ = nullptr;
};

AGZ_EDITOR_END
