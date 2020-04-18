#pragma once

#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include <agz/editor/common.h>
#include <agz/tracer/utility/config.h>

AGZ_EDITOR_BEGIN

class AssetLoader;
class AssetSaver;

class Transform2DUnitWidget : public QWidget
{
    Q_OBJECT

public:

    enum class UnitType
    {
        Translate,
        Rotate,
        Scale,
        None
    };

    using QWidget::QWidget;

    virtual UnitType get_type() const noexcept = 0;

    virtual tracer::Transform2 get_transform() const = 0;

    virtual Transform2DUnitWidget *clone() const = 0;

    virtual void save_asset(AssetSaver &saver) const = 0;

    virtual void load_asset(AssetLoader &loader) = 0;

    virtual RC<tracer::ConfigGroup> to_config() const = 0;

signals:

    void change_transform();

    void remove_this();
};

class Transform2DWidget : public QWidget
{
    Q_OBJECT

public:

    Transform2DWidget();

    tracer::Transform2 get_transform() const;

    Transform2DWidget *clone() const;

    void save_asset(AssetSaver &saver) const;

    void load_asset(AssetLoader &loader);

    RC<tracer::ConfigArray> to_config() const;

signals:

    void change_transform();

private:

    explicit Transform2DWidget(std::vector<Transform2DUnitWidget*> units);

    Transform2DUnitWidget::UnitType get_unit_type();

    Transform2DUnitWidget *create_new_unit_widget(
        Transform2DUnitWidget::UnitType type);

    void add_up();

    void add_down();

    void push_back(Transform2DUnitWidget *unit_widget);

    std::vector<Transform2DUnitWidget*> units_;

    QVBoxLayout *layout_ = nullptr;

    QPushButton *add_up_   = nullptr;
    QPushButton *add_down_ = nullptr;
};

AGZ_EDITOR_END
