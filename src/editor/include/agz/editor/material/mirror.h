#pragma once

#include <agz/editor/material/material.h>
#include <agz/editor/texture2d/texture2d.h>

AGZ_EDITOR_BEGIN

class MirrorWidget : public MaterialWidget
{
public:

    struct InitData
    {
        Texture2DSlot *color_map = nullptr;
        Texture2DSlot *eta       = nullptr;
        Texture2DSlot *k         = nullptr;
    };

    MirrorWidget(const InitData &init_data, ObjectContext &obj_ctx);

    ResourceWidget<tracer::Material> *clone() override;

    QPixmap get_thumbnail(int width, int height) const override;

protected:

    void update_tracer_object_impl() override;

private:

    void do_update_tracer_object();

    ObjectContext &obj_ctx_;

    Texture2DSlot *color_map_      = nullptr;
    Texture2DSlot *eta_            = nullptr;
    Texture2DSlot *k_              = nullptr;
};

class MirrorWidgetCreator : public MaterialWidgetCreator
{
public:

    QString name() const override
    {
        return "Mirror";
    }

    ResourceWidget<tracer::Material> *create_widget(ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
