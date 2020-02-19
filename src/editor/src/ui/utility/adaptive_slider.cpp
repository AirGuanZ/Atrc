#include <QVBoxLayout>

#include <agz/editor/ui/utility/adaptive_slider.h>

AGZ_EDITOR_BEGIN

RealSlider::RealSlider(QWidget *parent, double low, double high, double value)
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

    const double ratio = (value_ - low) / (high - low);
    const int slider_value = SLIDER_INT_MIN + static_cast<int>(std::floor(ratio * (SLIDER_INT_MAX - SLIDER_INT_MIN)));
    slider_->setValue(slider_value);

    connect(slider_, &QSlider::valueChanged, [=](int int_value)
    {
        const double t = (int_value - SLIDER_INT_MIN) / double(SLIDER_INT_MAX - SLIDER_INT_MIN);
        value_ = low_ * (1 - t) + high_ * t;
        emit change_value(value_);
    });
}

void RealSlider::set_orientation(Qt::Orientation orientation)
{
    slider_->setOrientation(orientation);
}

void RealSlider::set_range(double low, double high)
{
    low_   = low;
    high_  = high;
    value_ = math::clamp(value_, low, high);

    const double ratio = (value_ - low) / (high - low);
    const int slider_value = SLIDER_INT_MIN + static_cast<int>(std::floor(ratio * (SLIDER_INT_MAX - SLIDER_INT_MIN)));
    slider_->blockSignals(true);
    slider_->setValue(slider_value);
    slider_->blockSignals(false);
}

double RealSlider::value() const
{
    return value_;
}

void RealSlider::set_value(double value)
{
    value_ = math::clamp(value, low_, high_);

    const double ratio = (value_ - low_) / (high_ - low_);
    const int slider_value = SLIDER_INT_MIN + static_cast<int>(std::floor(ratio * (SLIDER_INT_MAX - SLIDER_INT_MIN)));
    slider_->blockSignals(true);
    slider_->setValue(slider_value);
    slider_->blockSignals(false);
}

double RealSlider::low() const
{
    return low_;
}

double RealSlider::high() const
{
    return high_;
}

AGZ_EDITOR_END
