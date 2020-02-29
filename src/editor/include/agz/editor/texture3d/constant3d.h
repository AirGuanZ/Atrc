#pragma once

#include <agz/editor/texture3d/texture3d.h>
#include <agz/editor/ui/utility/color_holder.h>
#include <agz/editor/ui/utility/vec_input.h>

AGZ_EDITOR_BEGIN

class Constant3DWidget : public Texture3DWidget
{
public:

    struct InitData
    {
        bool use_input_color        = false;
        Spectrum color_holder_value = Spectrum(real(0.5));
        Spectrum input_value        = Spectrum(real(0.5));
    };

    explicit Constant3DWidget(const InitData &init_data);

    Texture3DWidget *clone() override;

    std::unique_ptr<ResourceThumbnailProvider> get_thumbnail(int width, int height) const override;

    void save_asset(AssetSaver &saver) override;

    void load_asset(AssetLoader &loader) override;

protected:

    void update_tracer_object_impl() override;

private:

    QCheckBox *use_input_color_ = nullptr;

    SpectrumInput *input_color_  = nullptr;
    ColorHolder   *color_holder_ = nullptr;
};

class Constant3DWidgetCreator : public Texture3DWidgetCreator
{
public:

    QString name() const override { return "Constant"; }

    ResourceWidget<tracer::Texture3D> *create_widget(ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
