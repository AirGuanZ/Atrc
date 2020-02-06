#pragma once

#include <QSlider>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class AdaptiveSlider : public QWidget
{
    Q_OBJECT

public:

    explicit AdaptiveSlider(QWidget *parent);

    void set_orientation(Qt::Orientation orientation);

    void set_safe_slider_ratio(real slider_low, real slider_high);

    void set_range(real min_low, real max_high);

    void set_expected_range(real low, real high);

    real value() const;

    void set_value(real value);

signals:

    void change_value(real value);

private:

    void update_range();

    void update_slider();

    static int constexpr SLIDER_INT_MIN = 0;
    static int constexpr SLIDER_INT_MAX = 100000;

    real safe_slider_low_;
    real safe_slider_high_;

    real expected_low_;
    real expected_high_;

    real current_low_;
    real current_high_;

    real min_low_;
    real max_high_;

    real value_;

    QSlider *slider_;
};

AGZ_EDITOR_END
