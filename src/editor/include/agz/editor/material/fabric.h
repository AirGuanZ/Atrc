#pragma once

#include <agz/editor/material/material.h>
#include <agz/editor/material/normal_map.h>
#include <agz/editor/texture2d/texture2d.h>

AGZ_EDITOR_BEGIN

class FabricWidget : public MaterialWidget
{
public:

    struct InitData
    {
        Texture2DSlot *color     = nullptr;
        Texture2DSlot *roughness = nullptr;

        NormalMapWidget *normal_map = nullptr;
    };

    FabricWidget(const InitData &init_data, ObjectContext &obj_ctx);

    ResourceWidget<tracer::Material> *clone() override;

    Box<ResourceThumbnailProvider> get_thumbnail(
        int width, int height) const override;

    void save_asset(AssetSaver &saver) override;

    void load_asset(AssetLoader &loader) override;

    RC<tracer::ConfigNode> to_config(JSONExportContext &ctx) const override;

protected:

    void update_tracer_object_impl() override;

private:

    void do_update_tracer_object();

    ObjectContext &obj_ctx_;

    Texture2DSlot *color_     = nullptr;
    Texture2DSlot *roughness_ = nullptr;

    NormalMapWidget *normal_map_ = nullptr;
};

class FabricWidgetCreator : public MaterialWidgetCreator
{
public:

    QString name() const override
    {
        return "Fabric";
    }

    ResourceWidget<tracer::Material> *create_widget(
        ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
