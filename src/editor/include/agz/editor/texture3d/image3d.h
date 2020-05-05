#pragma once

#include <agz/editor/texture3d/texture3d_common_params.h>
#include <agz/editor/texture3d/texture3d.h>
#include <agz/editor/ui/utility/collapsible.h>
#include <agz/editor/ui/utility/elided_label.h>

AGZ_EDITOR_BEGIN

class Image3DWidget : public Texture3DWidget
{
    Q_OBJECT

public:

    struct InitData
    {
        RC<const Image3D<real>>          real_data;
        RC<const Image3D<Spectrum>>      spec_data;
        RC<const Image3D<uint8_t>>       uint8_data;
        RC<const Image3D<math::color3b>> uint24_data;

        QString filename;

        QString sampler_type = "Linear";

        Texture3DCommonParamsWidget *common = nullptr;
    };

    explicit Image3DWidget(const InitData &init_data);

    ResourceWidget<tracer::Texture3D> *clone() override;

    Box<ResourceThumbnailProvider> get_thumbnail(
        int width, int height) const override;

    void save_asset(AssetSaver &saver) override;

    void load_asset(AssetLoader &loader) override;

    RC<tracer::ConfigNode> to_config(JSONExportContext &ctx) const override;

protected:

    void update_tracer_object_impl() override;

private:

    void do_update_tracer_object();

    void browse_filename();

    RC<const Image3D<real>>          real_data_;
    RC<const Image3D<Spectrum>>      spec_data_;
    RC<const Image3D<uint8_t>>       uint8_data_;
    RC<const Image3D<math::color3b>> uint24_data_;

    QPushButton *browse_filename_ = nullptr;
    ElidedLabel *filename_        = nullptr;

    QComboBox *sampler_type_ = nullptr;

    Collapsible                 *common_section_ = nullptr;
    Texture3DCommonParamsWidget *common_         = nullptr;
};

class Image3DWidgetCreator : public Texture3DWidgetCreator
{
public:

    QString name() const override
    {
        return "Image3D";
    }

    ResourceWidget<tracer::Texture3D> *create_widget(
        ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
