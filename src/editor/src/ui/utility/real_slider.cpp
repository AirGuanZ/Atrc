#include <QVBoxLayout>

#include <agz/editor/ui/utility/real_slider.h>

AGZ_EDITOR_BEGIN

RealSlider::RealSlider(QWidget *parent, real low, real high, real value)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    slider_ = new QSlider(this);
    layout->addWidget(slider_);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    low_   = low;
    high_  = high;
    value_ = math::clamp(value, low, high);

    slider_->setRange(SLIDER_INT_MIN, SLIDER_INT_MAX);

    const real ratio = (value_ - low) / (high - low);
    const int slider_value = SLIDER_INT_MIN + static_cast<int>(std::floor(
        ratio * (SLIDER_INT_MAX - SLIDER_INT_MIN)));
    slider_->setValue(slider_value);

    connect(slider_, &QSlider::valueChanged, [=](int int_value)
    {
        const real t = (int_value - SLIDER_INT_MIN)
                     / real(SLIDER_INT_MAX - SLIDER_INT_MIN);
        value_ = low_ * (1 - t) + high_ * t;
        emit change_value(value_);
    });
}

void RealSlider::set_orientation(Qt::Orientation orientation)
{
    slider_->setOrientation(orientation);
}

void RealSlider::set_range(real low, real high)
{
    low_   = low;
    high_  = high;
    value_ = math::clamp(value_, low, high);

    const real ratio = (value_ - low) / (high - low);
    const int slider_value = SLIDER_INT_MIN + static_cast<int>(std::floor(
        ratio * (SLIDER_INT_MAX - SLIDER_INT_MIN)));
    slider_->blockSignals(true);
    slider_->setValue(slider_value);
    slider_->blockSignals(false);
}

real RealSlider::value() const
{
    return value_;
}

void RealSlider::set_value(real value)
{
    value_ = math::clamp(value, low_, high_);

    const real ratio = (value_ - low_) / (high_ - low_);
    const int slider_value = SLIDER_INT_MIN + static_cast<int>(std::floor(
        ratio * (SLIDER_INT_MAX - SLIDER_INT_MIN)));
    slider_->blockSignals(true);
    slider_->setValue(slider_value);
    slider_->blockSignals(false);
}

real RealSlider::low() const
{
    return low_;
}

real RealSlider::high() const
{
    return high_;
}

AGZ_EDITOR_END
