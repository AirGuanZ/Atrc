#pragma once

#include <QDoubleSpinBox>

#include <agz/editor/material/material.h>
#include <agz/editor/material/normal_map.h>
#include <agz/editor/texture2d/texture2d.h>
#include <agz/editor/ui/utility/real_slider.h>

AGZ_EDITOR_BEGIN

class PaperWidget : public MaterialWidget
{
public:

    struct InitData
    {
        Texture2DSlot *color = nullptr;

        real gf = real(0.335);
        real gb = real(-0.841);
        real wf = real(0.997);
        real front_eta = real(1.29);
        real back_eta  = real(1.55);
        real thickness = real(0.262);
        real sigma_s = real(81.38);
        real sigma_a = real(0.001);
        real front_roughness = real(0.419);
        real back_roughness  = real(0.892);

        NormalMapWidget *normal_map = nullptr;
    };

    PaperWidget(const InitData &init_data, ObjectContext &obj_ctx);

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

    Texture2DSlot *color_ = nullptr;

    QDoubleSpinBox *gf_ = nullptr;
    QDoubleSpinBox *gb_ = nullptr;
    QDoubleSpinBox *wf_ = nullptr;

    QDoubleSpinBox *front_eta_ = nullptr;
    QDoubleSpinBox *back_eta_  = nullptr;

    QDoubleSpinBox *thickness_ = nullptr;

    QDoubleSpinBox *sigma_s_ = nullptr;
    QDoubleSpinBox *sigma_a_ = nullptr;

    QDoubleSpinBox *front_roughness_ = nullptr;
    QDoubleSpinBox *back_roughness_ = nullptr;

    NormalMapWidget *normal_map_ = nullptr;
};

class PaperWidgetCreator : public MaterialWidgetCreator
{
public:

    QString name() const override
    {
        return "Paper";
    }

    ResourceWidget<tracer::Material> *create_widget(
        ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
