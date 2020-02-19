#pragma once

#include <agz/editor/material/material.h>
#include <agz/editor/material/normal_map.h>
#include <agz/editor/texture2d/texture2d.h>

AGZ_EDITOR_BEGIN

class DisneyWidget : public MaterialWidget
{
public:

    struct InitData
    {
        Texture2DSlot *base_color             = nullptr;
        Texture2DSlot *metallic               = nullptr;
        Texture2DSlot *roughness              = nullptr;
        Texture2DSlot *transmission           = nullptr;
        Texture2DSlot *transmission_roughness = nullptr;
        Texture2DSlot *ior                    = nullptr;
        Texture2DSlot *specular_scale         = nullptr;
        Texture2DSlot *specular_tint          = nullptr;
        Texture2DSlot *anisotropic            = nullptr;
        Texture2DSlot *sheen                  = nullptr;
        Texture2DSlot *sheen_tint             = nullptr;
        Texture2DSlot *clearcoat              = nullptr;
        Texture2DSlot *clearcoat_gloss        = nullptr;
        NormalMapWidget *normal_map           = nullptr;
    };

    DisneyWidget(const InitData &init_data, ObjectContext &obj_ctx);

    ResourceWidget<tracer::Material> *clone() override;

    QPixmap get_thumbnail(int width, int height) const override;

protected:

    void update_tracer_object_impl() override;

private:

    void do_update_tracer_object();

    ObjectContext &obj_ctx_;
    
    Texture2DSlot *base_color_             = nullptr;
    Texture2DSlot *metallic_               = nullptr;
    Texture2DSlot *roughness_              = nullptr;
    Texture2DSlot *transmission_           = nullptr;
    Texture2DSlot *transmission_roughness_ = nullptr;
    Texture2DSlot *ior_                    = nullptr;
    Texture2DSlot *specular_scale_         = nullptr;
    Texture2DSlot *specular_tint_          = nullptr;
    Texture2DSlot *anisotropic_            = nullptr;
    Texture2DSlot *sheen_                  = nullptr;
    Texture2DSlot *sheen_tint_             = nullptr;
    Texture2DSlot *clearcoat_              = nullptr;
    Texture2DSlot *clearcoat_gloss_        = nullptr;
    NormalMapWidget *normal_map_           = nullptr;
};

class DisneyWidgetCreator : public MaterialWidgetCreator
{
public:

    QString name() const override
    {
        return "Disney";
    }

    ResourceWidget<tracer::Material> *create_widget(ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
