#pragma once

#include <QPushButton>
#include <QVBoxLayout>

#include <agz/editor/ui/utility/vec_input.h>
#include <agz/editor/utility/direct_transform3d.h>

AGZ_EDITOR_BEGIN

class AssetLoader;
class AssetSaver;

class Transform3DSeqUnitWidget : public QWidget
{
    Q_OBJECT

public:

    enum class UnitType : uint8_t
    {
        Translate,
        RotateX,
        RotateY,
        RotateZ,
        Rotate,
        Scale
    };

    using QWidget::QWidget;

    virtual UnitType get_type() const noexcept = 0;

    virtual tracer::Transform3 get_transform() const = 0;

    virtual Transform3DSeqUnitWidget *clone() const = 0;

    virtual void save_asset(AssetSaver &saver) const = 0;

    virtual void load_asset(AssetLoader &loader) = 0;

signals:

    void change_transform();

    void remove_this();
};

class Transform3DSeqWidget : public QWidget
{
    Q_OBJECT

public:

    Transform3DSeqWidget();

    tracer::Transform3 get_transform() const;

    Transform3DSeqWidget *clone() const;

    void save_asset(AssetSaver &saver) const;

    void load_asset(AssetLoader &loader);

signals:

    void change_transform();

private:

    explicit Transform3DSeqWidget(std::vector<Transform3DSeqUnitWidget *> units);

    Transform3DSeqUnitWidget::UnitType get_unit_type();

    Transform3DSeqUnitWidget *create_new_unit_widget(Transform3DSeqUnitWidget::UnitType type);

    void add_up();

    void add_down();

    void push_back(Transform3DSeqUnitWidget *unit_widget);

    std::vector<Transform3DSeqUnitWidget *> units_;

    QVBoxLayout *layout_ = nullptr;

    QPushButton *add_up_ = nullptr;
    QPushButton *add_down_ = nullptr;
};

AGZ_EDITOR_END
