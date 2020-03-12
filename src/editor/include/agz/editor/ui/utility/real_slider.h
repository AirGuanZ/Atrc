#pragma once

#include <QSlider>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class RealSlider : public QWidget
{
    Q_OBJECT

public:

    explicit RealSlider(
        QWidget *parent, double low = 0, double high = 1, double value = 0);

    void set_orientation(Qt::Orientation orientation);

    void set_range(double low, double high);

    double value() const;

    void set_value(double value);

    double low() const;

    double high() const;

signals:

    void change_value(double value);

private:

    static int constexpr SLIDER_INT_MIN = 0;
    static int constexpr SLIDER_INT_MAX = 100000;

    double low_;
    double high_;

    double value_;

    QSlider *slider_;
};

AGZ_EDITOR_END
