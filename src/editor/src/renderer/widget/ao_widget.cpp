#include <QColorDialog>

#include <agz/editor/renderer/ao.h>
#include <agz/editor/renderer/widget/ao_widget.h>

#include "ui_AO.h"

AGZ_EDITOR_BEGIN

AOWidget::AOWidget(QWidget *parent)
    : RendererWidget(parent), ui_(new Ui::AO)
{
    ui_->setupUi(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(ui_->formLayoutWidget);

    ui_->sample_count_spinbox->setValue(ao_sample_count_);
    ui_->occlusion_distance_spinbox->setValue(occlusion_distance_);

    set_button_color(ui_->background_color, background_color_);
    set_button_color(ui_->low_color, low_color_);
    set_button_color(ui_->high_color, high_color_);

    using IntPtr = void(QSpinBox:: *)(int);
    const IntPtr int_ptr = &QSpinBox::valueChanged;
    connect(ui_->sample_count_spinbox, int_ptr, [=](int new_value)
    {
        ao_sample_count_ = new_value;
        emit change_renderer_params();
    });

    using DoublePtr = void(QDoubleSpinBox:: *)(double);
    const DoublePtr double_ptr = &QDoubleSpinBox::valueChanged;
    connect(ui_->occlusion_distance_spinbox, double_ptr, [=](double new_value)
    {
        occlusion_distance_ = static_cast<real>(new_value);
        emit change_renderer_params();
    });

    connect(ui_->background_color, &QPushButton::clicked, [=]
    {
        QColor new_color = QColorDialog::getColor(background_color_, this);
        if(new_color.isValid())
        {
            background_color_ = new_color;
            set_button_color(ui_->background_color, background_color_);
            emit change_renderer_params();
        }
    });

    connect(ui_->low_color, &QPushButton::clicked, [=]
    {
        QColor new_color = QColorDialog::getColor(low_color_, this);
        if(new_color.isValid())
        {
            low_color_ = new_color;
            set_button_color(ui_->low_color, low_color_);
            emit change_renderer_params();
        }
    });

    connect(ui_->high_color, &QPushButton::clicked, [=]
    {
        QColor new_color = QColorDialog::getColor(high_color_, this);
        if(new_color.isValid())
        {
            high_color_ = new_color;
            set_button_color(ui_->high_color, high_color_);
            emit change_renderer_params();
        }
    });
}

Box<Renderer> AOWidget::create_renderer(
    RC<tracer::Scene> scene, const Vec2i &framebuffer_size,
    bool enable_preview) const
{
    const AO::Params params = {
        -2, 32, ao_sample_count_, occlusion_distance_,
        qcolor_to_spectrum(background_color_),
        qcolor_to_spectrum(low_color_),
        qcolor_to_spectrum(high_color_),
        enable_preview
    };
    return newBox<AO>(
        params, framebuffer_size.x, framebuffer_size.y, std::move(scene));
}

void AOWidget::set_button_color(QPushButton *button, const QColor &color)
{
    button->setAutoFillBackground(true);
    QPalette pal = button->palette();
    pal.setColor(QPalette::Button, color);
    button->setPalette(pal);
}

RendererWidget *AOWidgetCreator::create_widget(QWidget *parent) const
{
    return new AOWidget(parent);
}

AGZ_EDITOR_END
