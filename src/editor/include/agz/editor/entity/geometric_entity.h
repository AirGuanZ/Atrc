#pragma once

#include <agz/editor/entity/entity.h>
#include <agz/editor/geometry/geometry.h>
#include <agz/editor/material/material.h>
#include <agz/editor/medium/medium.h>
#include <agz/editor/ui/utility/vec_input.h>
#include <agz/editor/ui/transform3d_widget.h>

AGZ_EDITOR_BEGIN

class GeometricEntityWidget : public EntityWidget
{
public:

    struct CloneState
    {
        GeometrySlot *geometry = nullptr;
        MaterialSlot *material = nullptr;
        MediumSlot *medium_in  = nullptr;
        MediumSlot *medium_out = nullptr;
        Spectrum emit_radiance;
        DirectTransform transform;
    };

    GeometricEntityWidget(const CloneState &clone_state, ObjectContext &obj_ctx);

    ResourceWidget<tracer::Entity> *clone() override;

    void save_asset(AssetSaver &saver) override;

    void load_asset(AssetLoader &loader) override;

    RC<tracer::ConfigNode> to_config(JSONExportContext &ctx) const override;

    std::vector<Vertex> get_vertices() const override;

    DirectTransform get_transform() const override;

    void set_transform(const DirectTransform &transform) override;

protected:

    void update_tracer_object_impl() override;

private:

    void do_update_tracer_object();

    ObjectContext &obj_ctx_;

    GeometrySlot *geometry_ = nullptr;
    MaterialSlot *material_ = nullptr;
    MediumSlot *medium_in_  = nullptr;
    MediumSlot *medium_out_ = nullptr;

    SpectrumInput *emit_radiance_ = nullptr;

    Transform3DWidget *transform_ = nullptr;
};

class GeometricEntityWidgetCreator : public EntityWidgetCreator
{
public:

    QString name() const override
    {
        return "Geometric";
    }

    ResourceWidget<tracer::Entity> *create_widget(
        ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
