#pragma once

#include <agz/editor/envir_light/envir_light.h>
#include <agz/editor/ui/utility/color_holder.h>

AGZ_EDITOR_BEGIN

class NativeSkyWidget : public EnvirLightWidget
{
    Q_OBJECT

public:

    struct CloneData
    {
        Spectrum top    = Spectrum(1);
        Spectrum bottom = Spectrum(0);
    };

    explicit NativeSkyWidget(const CloneData &clone_data);

    ResourceWidget<tracer::EnvirLight> *clone() override;

    void save_asset(AssetSaver &saver) override;

    void load_asset(AssetLoader &loader) override;

    std::shared_ptr<tracer::ConfigNode> to_config(JSONExportContext &ctx) const override;

protected:

    void update_tracer_object_impl() override;

private:

    void do_update_tracer_object();

    ColorHolder *top_     = nullptr;
    ColorHolder *bottom_  = nullptr;
};

class NativeSkyCreator : public EnvirLightWidgetCreator
{
public:

    QString name() const override { return "Native Sky"; }

    ResourceWidget<tracer::EnvirLight> *create_widget(ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
