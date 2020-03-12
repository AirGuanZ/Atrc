#pragma once

#include <QDoubleSpinBox>

#include <agz/editor/texture3d/texture3d.h>
#include <agz/editor/ui/utility/real_slider.h>
#include <agz/editor/ui/utility/validator.h>

AGZ_EDITOR_BEGIN

class Range3DWidget : public Texture3DWidget
{
    Q_OBJECT

public:

    struct CloneState
    {
        real low   = 0;
        real high  = 1;
        real value = 0;
    };

    explicit Range3DWidget(const CloneState &clone_state);

    ResourceWidget<tracer::Texture3D> *clone() override;

    Box<ResourceThumbnailProvider> get_thumbnail(
        int width, int height) const override;

    void save_asset(AssetSaver &saver) override;

    void load_asset(AssetLoader &loader) override;

    RC<tracer::ConfigNode> to_config(JSONExportContext &ctx) const override;

protected:

    void update_tracer_object_impl() override;

private:

    void set_range(double low, double high);

    void do_update_tracer_object();

    Box<RealRangeValidator> range_edit_validator_;

    QLineEdit      *range_edit_ = nullptr;
    QDoubleSpinBox *value_      = nullptr;

    RealSlider *slider_ = nullptr;
};

class Range3DWidgetCreator : public Texture3DWidgetCreator
{
public:

    QString name() const override
    {
        return "Range";
    }

    ResourceWidget<tracer::Texture3D> *create_widget(
        ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
