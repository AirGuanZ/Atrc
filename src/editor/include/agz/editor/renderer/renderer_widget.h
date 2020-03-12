#pragma once

#include <map>
#include <memory>

#include <QComboBox>
#include <QVBoxLayout>

#include <agz/editor/renderer/export/export_renderer.h>
#include <agz/editor/renderer/renderer.h>

AGZ_EDITOR_BEGIN

class RendererWidget : public QWidget
{
    Q_OBJECT

public:

    using QWidget::QWidget;

    virtual ~RendererWidget() = default;

    virtual Box<Renderer> create_renderer(
        RC<tracer::Scene> scene, const Vec2i &framebuffer_size,
        bool enable_preview) const = 0;

signals:

    void change_renderer_params();
};

class RendererWidgetCreator : public misc::uncopyable_t
{
public:

    virtual ~RendererWidgetCreator() = default;

    virtual std::string name() const = 0;

    virtual RendererWidget *create_widget(QWidget *parent) const = 0;
};

class RendererWidgetFactory : public misc::uncopyable_t
{
public:

    void add_creator(Box<RendererWidgetCreator> creator);

    RendererWidget *create_widget(std::string_view name, QWidget *parent) const;

    std::vector<std::string> get_type_names() const;

private:

    std::map<std::string, Box<RendererWidgetCreator>, std::less<>> name2creator_;
};

class RendererPanel : public QWidget
{
    Q_OBJECT

public:

    RendererPanel(QWidget *parent, const std::string &default_renderer_type);

    Box<Renderer> create_renderer(
        RC<tracer::Scene> scene, const Vec2i &framebuffer_size,
        bool enable_preview) const;

    RC<tracer::ConfigGroup> to_config() const;

signals:

    void change_renderer_type();

    void change_renderer_params();

private slots:

    void on_change_renderer_params();

private:

    // preview

    QComboBox      *renderer_type_selector_;
    QVBoxLayout    *layout_;
    RendererWidget *renderer_widget_;

    RendererWidgetFactory factory_;

    // export

    ExportRendererPanel *export_renderer_;
};

AGZ_EDITOR_END
