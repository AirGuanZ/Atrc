#pragma once

#include <agz/editor/material/material.h>
#include <agz/editor/material/normal_map.h>
#include <agz/editor/texture2d/texture2d.h>

AGZ_EDITOR_BEGIN

class IdealDiffuseWidget : public MaterialWidget
{
public:

    struct CloneState
    {
        Texture2DSlot *albedo       = nullptr;
        NormalMapWidget *normal_map = nullptr;
    };

    explicit IdealDiffuseWidget(const CloneState &clone_state, ObjectContext &obj_ctx);

    ResourceWidget<tracer::Material> *clone() override;

    QPixmap get_thumbnail(int width, int height) const override;

protected:

    void update_tracer_object_impl() override;

private:

    void do_update_tracer_object();

    ObjectContext &obj_ctx_;

    Texture2DSlot *albedo_ = nullptr;

    NormalMapWidget *normal_map_ = nullptr;
};

class IdealDiffuseWidgetCreator : public MaterialWidgetCreator
{
public:

    QString name() const override
    {
        return "Ideal Diffuse";
    }

    ResourceWidget<tracer::Material> *create_widget(ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
