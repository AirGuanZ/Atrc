#pragma once

#include <agz/editor/material/material.h>
#include <agz/editor/texture2d/texture2d.h>

AGZ_EDITOR_BEGIN

class GlassWidget : public MaterialWidget
{
public:

    struct InitData
    {
        Texture2DSlot *color = nullptr;
        Texture2DSlot *ior   = nullptr;

        bool use_color_refr = false;
        Texture2DSlot *color_refr = nullptr;
    };

    GlassWidget(const InitData &init_data, ObjectContext &obj_ctx);

    ResourceWidget<tracer::Material> *clone() override;

    Box<ResourceThumbnailProvider> get_thumbnail(
        int width, int height) const override;

    void save_asset(AssetSaver &saver) override;

    void load_asset(AssetLoader &loader) override;

    RC<tracer::ConfigNode> to_config(
        JSONExportContext &ctx) const override;

protected:

    void update_tracer_object_impl() override;

private:

    void do_update_tracer_object();

    ObjectContext &obj_ctx_;

    Texture2DSlot *color_ = nullptr;
    Texture2DSlot *ior_   = nullptr;

    QCheckBox     *use_color_refr_ = nullptr;
    Texture2DSlot *color_refr_     = nullptr;
};

class GlassWidgetCreator : public MaterialWidgetCreator
{
public:

    QString name() const override
    {
        return "Glass";
    }

    ResourceWidget<tracer::Material> *create_widget(
        ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
