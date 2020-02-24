#include <QEvent>

#include <agz/editor/texture2d/range.h>

AGZ_EDITOR_BEGIN

RangeWidget::RangeWidget(const CloneState &clone_state)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QWidget     *hwidget = new QWidget(this);
    QHBoxLayout *hlayout = new QHBoxLayout(hwidget);

    range_edit_validator_ = std::make_unique<RealRangeValidator>();

    range_edit_ = new QLineEdit(hwidget);
    value_      = new QDoubleSpinBox(hwidget);

    slider_ = new RealSlider(this);
    slider_->set_orientation(Qt::Horizontal);

    range_edit_->setAlignment(Qt::AlignCenter);
    range_edit_->setText(QString("%1 %2").arg(clone_state.low).arg(clone_state.high));
    range_edit_->setValidator(range_edit_validator_.get());

    hlayout->addWidget(value_);
    hlayout->addWidget(range_edit_);

    layout->addWidget(hwidget);
    layout->addWidget(slider_);

    hwidget->setContentsMargins(0, 0, 0, 0);
    hlayout->setContentsMargins(0, 0, 0, 0);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    value_->setButtonSymbols(QDoubleSpinBox::NoButtons);

    value_->setRange(clone_state.low, clone_state.high);
    value_->setValue(clone_state.value);

    slider_->set_range(clone_state.low, clone_state.high);
    slider_->set_value(clone_state.value);

    class DisableSliderWheel : public QObject
    {
    protected:

        using QObject::QObject;

        bool eventFilter(QObject *watched, QEvent *event) override
        {
            if(event->type() == QEvent::Wheel)
                return true;
            return QObject::eventFilter(watched, event);
        }
    };
    slider_->installEventFilter(new DisableSliderWheel(slider_));

    connect(range_edit_, &QLineEdit::returnPressed, [=]
    {
        QString text = range_edit_->text();
        auto [low, high] = RealRangeValidator::parse(text);
        set_range(low, high);
    });

    connect(value_, qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double new_value)
    {
        slider_->set_value(new_value);
        set_dirty_flag();
    });

    connect(slider_, &RealSlider::change_value, [=](double new_value)
    {
        value_->blockSignals(true);
        value_->setValue(new_value);
        value_->blockSignals(false);
        set_dirty_flag();
    });

    do_update_tracer_object();
}

ResourceWidget<tracer::Texture2D> *RangeWidget::clone()
{
    CloneState clone_state;
    clone_state.low   = slider_->low();
    clone_state.high  = slider_->high();
    clone_state.value = slider_->value();
    return new RangeWidget(clone_state);
}

QPixmap RangeWidget::get_thumbnail(int width, int height) const
{
    const double value = slider_->value();

    QImage img(1, 1, QImage::Format::Format_RGB888);
    img.setPixelColor(0, 0, QColor::fromRgbF(value, value, value));

    QPixmap pixmap;
    pixmap.convertFromImage(img);

    return pixmap.scaled(width, height);
}

void RangeWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void RangeWidget::set_range(double low, double high)
{
    value_->blockSignals(true);
    const double new_value = math::clamp(value_->value(), low, high);
    if(new_value != value_->value())
    {
        value_->setValue(new_value);
        slider_->set_value(new_value);
        set_dirty_flag();
    }
    value_->blockSignals(false);

    value_->setRange(low, high);
    slider_->set_range(low, high);
}

void RangeWidget::do_update_tracer_object()
{
    tracer_object_ = tracer::create_constant2d_texture({}, slider_->value());
}

ResourceWidget<tracer::Texture2D> *RangeWidgetCreator::create_widget(ObjectContext &obj_ctx) const
{
    return new RangeWidget({});
}

AGZ_EDITOR_END
