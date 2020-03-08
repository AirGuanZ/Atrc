#pragma once

#include <QCheckBox>

#include <agz/editor/texture2d/texture2d.h>
#include <agz/editor/ui/utility/color_holder.h>
#include <agz/editor/ui/utility/vec_input.h>

AGZ_EDITOR_BEGIN

class Constant2DWidget : public Texture2DWidget
{
public:

    struct InitData
    {
        bool use_input_color        = false;
        Spectrum color_holder_value = Spectrum(real(0.5));
        Spectrum input_value        = Spectrum(real(0.5));
    };

    explicit Constant2DWidget(const InitData &init_data);

    Texture2DWidget *clone() override;

    std::unique_ptr<ResourceThumbnailProvider> get_thumbnail(int width, int height) const override;

    void save_asset(AssetSaver &saver) override;

    void load_asset(AssetLoader &loader) override;

    std::shared_ptr<tracer::ConfigNode> to_config(JSONExportContext &ctx) const override;

protected:

    void update_tracer_object_impl() override;

private:

    QCheckBox *use_input_color_ = nullptr;

    SpectrumInput *input_color_  = nullptr;
    ColorHolder   *color_holder_ = nullptr;
};

class Constant2DCreator : public Texture2DWidgetCreator
{
public:

    QString name() const override { return "Constant"; }

    ResourceWidget<tracer::Texture2D> *create_widget(ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
