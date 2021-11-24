#pragma once

#include <agz/editor/texture2d/texture2d.h>
#include <agz/editor/texture3d/texture3d.h>

AGZ_EDITOR_BEGIN

class SolidImage2DWidget : public Texture2DWidget
{
    Q_OBJECT

public:

    struct InitData
    {
        Texture3DSlot *tex3d = nullptr;
    };

    SolidImage2DWidget(const InitData &clone_data, ObjectContext &object_context);

    ResourceWidget<tracer::Texture2D> *clone() override;

    Box<ResourceThumbnailProvider> get_thumbnail(int width, int height) const override;

    void save_asset(AssetSaver &saver) override;

    void load_asset(AssetLoader &loader) override;

    RC<tracer::ConfigNode> to_config(JSONExportContext &ctx) const override;

protected:

    void update_tracer_object_impl() override;

private:

    void do_update_tracer_object();

    ObjectContext &object_context_;
    Texture3DSlot *tex3d_ = nullptr;
};

class SolidImage2DCreator : public Texture2DWidgetCreator
{
public:

    QString name() const override { return "SolidImage"; }

    ResourceWidget<tracer::Texture2D> *create_widget(
        ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
