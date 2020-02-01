#pragma once

#include <map>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class Texture2DWidget : public QWidget
{
    Q_OBJECT

public:

    using QWidget::QWidget;

    virtual ~Texture2DWidget() = default;

    virtual std::shared_ptr<tracer::Texture2D> create_texture2d() const = 0;

signals:

    void change_texture2d_params();
};

class Texture2DWidgetCreator : public misc::uncopyable_t
{
public:

    virtual ~Texture2DWidgetCreator() = default;

    virtual std::string name() const = 0;

    virtual Texture2DWidget *create_widget(QWidget *parent) const = 0;
};

class Texture2DWidgetFactory : public misc::uncopyable_t
{
public:

    void add_creator(std::unique_ptr<Texture2DWidgetCreator> creator);

    Texture2DWidget *create_widget(std::string_view name, QWidget *parent) const;

    std::vector<std::string> get_type_names() const;

private:

    std::map<std::string, std::unique_ptr<Texture2DWidget>, std::less<>> name2creator_;
};

// TODO

AGZ_EDITOR_END
