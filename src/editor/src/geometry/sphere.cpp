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

    connect(radius_edit_, &QLineEdit::returnPressed, [=]
    {
        set_geometry_vertices_dirty();
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

std::vector<EntityInterface::Vertex> SphereWidget::get_vertices() const
{
    static const auto UNIT_VERTICES = unit_vertices();
    auto ret = UNIT_VERTICES;
    for(auto &v : ret)
        v.pos *= radius_edit_->text().toFloat();
    return ret;
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

std::vector<EntityInterface::Vertex> SphereWidget::unit_vertices()
{
    std::vector<Vertex> ret;

    constexpr int X_GRID_COUNT = 32;
    constexpr int Y_GRID_COUNT = 32;

    const real DELTA_X_RAD = 2 * PI_r / X_GRID_COUNT;
    const real DELTA_Y_RAD = PI_r / Y_GRID_COUNT;

    auto rad_to_pos = [](real x_rad, real y_rad)
    {
        return Vec3{
            std::cos(y_rad) * std::cos(x_rad),
            std::cos(y_rad) * std::sin(x_rad),
            std::sin(y_rad)
        };
    };

    auto add_pos = [&](const Vec3 &pos)
    {
        ret.push_back({ pos, pos.normalize() });
    };

    real y_rad = -PI_r / 2 + DELTA_Y_RAD;
    real x_rad = 0;
    for(int xi = 0; xi < X_GRID_COUNT; ++xi)
    {
        const real next_x_rad = x_rad + DELTA_X_RAD;

        add_pos({ 0, 0, -1 });
        add_pos(rad_to_pos(x_rad, y_rad));
        add_pos(rad_to_pos(next_x_rad, y_rad));

        x_rad = next_x_rad;
    }

    for(int yi = 1; yi < Y_GRID_COUNT - 1; ++yi)
    {
        const real next_y_rad = y_rad + DELTA_Y_RAD;

        x_rad = 0;
        for(int xi = 0; xi < X_GRID_COUNT; ++xi)
        {
            const real next_x_rad = x_rad + DELTA_X_RAD;

            const Vec3 lb = rad_to_pos(x_rad, y_rad);
            const Vec3 rb = rad_to_pos(next_x_rad, y_rad);
            const Vec3 lt = rad_to_pos(x_rad, next_y_rad);
            const Vec3 rt = rad_to_pos(next_x_rad, next_y_rad);

            add_pos(lb); add_pos(lt); add_pos(rt);
            add_pos(lb); add_pos(rt); add_pos(rb);

            x_rad = next_x_rad;
        }

        y_rad = next_y_rad;
    }

    x_rad = 0;
    for(int xi = 0; xi < X_GRID_COUNT; ++xi)
    {
        const real next_x_rad = x_rad + DELTA_X_RAD;

        add_pos(rad_to_pos(x_rad, y_rad));
        add_pos({ 0, 0, 1 });
        add_pos(rad_to_pos(next_x_rad, y_rad));

        x_rad = next_x_rad;
    }

    return ret;
}

AGZ_EDITOR_END
