#include <agz/editor/geometry/sphere.h>

AGZ_EDITOR_BEGIN

SphereWidget::SphereWidget(const CloneState &clone_state)
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    layout->addWidget(new QLabel("Radius", this));

    radius_edit_validator_ = std::make_unique<QDoubleValidator>();
    radius_edit_ = new QLineEdit(this);

    radius_edit_->setText(QString::number(clone_state.radius));
    radius_edit_->setValidator(radius_edit_validator_.get());
    radius_edit_->setAlignment(Qt::AlignCenter);

    layout->addWidget(radius_edit_);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    connect(radius_edit_, &QLineEdit::textChanged, [=](const QString&)
    {
        set_dirty_flag();
    });

    do_update_tracer_object();
}

ResourceWidget<tracer::Geometry> *SphereWidget::clone()
{
    CloneState clone_state;
    clone_state.radius = radius_edit_->text().toFloat();
    return new SphereWidget(clone_state);
}

QPixmap SphereWidget::get_thumbnail(int width, int height) const
{
    return QPixmap(width, height);
}

void SphereWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void SphereWidget::do_update_tracer_object()
{
    const real radius = radius_edit_->text().toFloat();
    tracer_object_ = tracer::create_sphere(radius, {});
}

ResourceWidget<tracer::Geometry> *SphereWidgetCreator::create_widget(ObjectContext &obj_ctx) const
{
    return new SphereWidget({});
}

AGZ_EDITOR_END
