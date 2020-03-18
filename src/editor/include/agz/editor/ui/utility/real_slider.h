#pragma once

#include <QSlider>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class RealSlider : public QWidget
{
    Q_OBJECT

public:

    explicit RealSlider(
        QWidget *parent, real low = 0, real high = 1, real value = 0);

    void set_orientation(Qt::Orientation orientation);

    void set_range(real low, real high);

    real value() const;

    void set_value(real value);

    real low() const;

    real high() const;

signals:

    void change_value(real value);

private:

    static int constexpr SLIDER_INT_MIN = 0;
    static int constexpr SLIDER_INT_MAX = 100000;

    real low_;
    real high_;

    real value_;

    QSlider *slider_;
};

AGZ_EDITOR_END
