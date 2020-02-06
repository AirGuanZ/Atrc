#include <QVBoxLayout>

#include <agz/editor/ui/utility/adaptive_slider.h>

AGZ_EDITOR_BEGIN

AdaptiveSlider::AdaptiveSlider(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    slider_ = new QSlider(this);
    layout->addWidget(slider_);

    safe_slider_low_  = real(0.2);
    safe_slider_high_ = real(0.8);

    expected_low_  = 0;
    expected_high_ = 1;

    current_low_  = real(0);
    current_high_ = real(1);

    min_low_  = real(0);
    max_high_ = real(1);

    value_ = 0;

    slider_->setRange(SLIDER_INT_MIN, SLIDER_INT_MAX);

    update_range();

    connect(slider_, &QSlider::valueChanged, [=](int v)
    {
        const real ratio = (v - SLIDER_INT_MIN) / real(SLIDER_INT_MAX - SLIDER_INT_MIN);
        value_ = math::lerp(current_low_, current_high_, ratio);

        emit change_value(value_);
    });

    connect(slider_, &QSlider::sliderReleased, [=]
    {
        update_range();
    });
}

void AdaptiveSlider::set_orientation(Qt::Orientation orientation)
{
    slider_->setOrientation(orientation);
}

void AdaptiveSlider::set_safe_slider_ratio(real slider_low, real slider_high)
{
    safe_slider_low_  = slider_low;
    safe_slider_high_ = slider_high;

    update_range();
}

void AdaptiveSlider::set_range(real min_low, real max_high)
{
    min_low_  = min_low;
    max_high_ = max_high;

    expected_low_  = math::clamp(expected_low_,  min_low, max_high);
    expected_high_ = math::clamp(expected_high_, min_low, max_high);
    value_         = math::clamp(value_,         min_low, max_high);

    update_range();
}

void AdaptiveSlider::set_expected_range(real low, real high)
{
    expected_low_  = math::clamp(low,  min_low_, max_high_);
    expected_high_ = math::clamp(high, min_low_, max_high_);

    update_range();
}

real AdaptiveSlider::value() const
{
    return value_;
}

void AdaptiveSlider::set_value(real value)
{
    value_ = math::clamp(value, min_low_, max_high_);

    update_range();
}

void AdaptiveSlider::update_range()
{
    // if value is out of safe ratio range, try to expand current_range

    const real current_slider_ratio = (value_ - current_low_) / (current_high_ - current_low_);
    if(current_slider_ratio < safe_slider_low_)
    {
        // solve (value - x) / (current_high - x) = safe_slider_low for x

        const real expected_low = (current_high_ * safe_slider_low_ - value_) / (safe_slider_low_ - 1);
        current_low_ = (std::max)(expected_low, min_low_);
    }
    else if(current_slider_ratio > safe_slider_high_)
    {
        // solve (value - current_low) / (x - current_low) = safe_slider_high for x

        const real expected_high = current_low_ + (value_ - current_low_) / safe_slider_high_;
        current_high_ = (std::min)(expected_high, max_high_);
    }
    else
    {
        // if value is in safe ratio range, current_range may be too large
        // try to shrink it to expected_range

        // TODO
    }

    slider_->blockSignals(true);
    update_slider();
    slider_->blockSignals(false);
}

void AdaptiveSlider::update_slider()
{
    const real ratio = (value_ - current_low_) / current_high_;
    const int slider_value = SLIDER_INT_MIN + static_cast<int>(std::floor(ratio * (SLIDER_INT_MAX - SLIDER_INT_MIN)));
    slider_->setValue(slider_value);
}

AGZ_EDITOR_END
