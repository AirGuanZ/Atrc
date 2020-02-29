#pragma once

#include <agz/editor/entity/entity.h>
#include <agz/editor/geometry/geometry.h>
#include <agz/editor/ui/transform3d_widget.h>

AGZ_EDITOR_BEGIN

class DiffuseLightEntityWidget : public EntityWidget
{
public:

    struct CloneState
    {
        GeometrySlot *geometry = nullptr;
        Spectrum      radiance = Spectrum(1);

        DirectTransform transform;
    };

    DiffuseLightEntityWidget(const CloneState &clone_state, ObjectContext &obj_ctx);

    ResourceWidget<tracer::Entity> *clone() override;

    void save_asset(AssetSaver &saver) override;

    void load_asset(AssetLoader &loader) override;

    std::vector<Vertex> get_vertices() const override;

    DirectTransform get_transform() const override;

    void set_transform(const DirectTransform &transform) override;

protected:

    void update_tracer_object_impl() override;

private:

    void do_update_tracer_object();

    ObjectContext &obj_ctx_;

    GeometrySlot  *geometry_ = nullptr;
    SpectrumInput *radiance_ = nullptr;

    Transform3DWidget *transform_ = nullptr;
};

class DiffuseLightEntityWidgetCreator : public EntityWidgetCreator
{
public:

    QString name() const override
    {
        return "Diffuse Light";
    }

    ResourceWidget<tracer::Entity> *create_widget(ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
