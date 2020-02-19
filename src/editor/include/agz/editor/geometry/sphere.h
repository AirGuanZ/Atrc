#pragma once

#include <agz/editor/geometry/geometry.h>

AGZ_EDITOR_BEGIN

class SphereWidget : public GeometryWidget
{
    Q_OBJECT

public:

    struct CloneState
    {
        real radius = 1;
    };

    explicit SphereWidget(const CloneState &clone_state);

    ResourceWidget<tracer::Geometry> *clone() override;

    QPixmap get_thumbnail(int width, int height) const override;

protected:

    void update_tracer_object_impl() override;

private:

    void do_update_tracer_object();

    QLineEdit *radius_edit_ = nullptr;
    std::unique_ptr<QValidator> radius_edit_validator_;
};

class SphereWidgetCreator : public GeometryWidgetCreator
{
public:

    QString name() const override { return "Sphere"; }

    ResourceWidget<tracer::Geometry> *create_widget(ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
