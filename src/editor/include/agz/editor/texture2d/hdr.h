#pragma once

#include <QCheckBox>
#include <QDoubleSpinBox>

#include <agz/editor/texture2d/texture2d_common_params.h>
#include <agz/editor/texture2d/texture2d.h>
#include <agz/editor/ui/transform2d_widget.h>
#include <agz/editor/ui/utility/collapsible.h>
#include <agz/editor/ui/utility/elided_label.h>

AGZ_EDITOR_BEGIN

class HDRWidget : public Texture2DWidget
{
    Q_OBJECT

public:

    struct CloneState
    {
        QString filename;
        std::shared_ptr<const Image2D<math::color3f>> img_data;

        Texture2DCommonParamsWidget *adv = nullptr;
    };

    explicit HDRWidget(const CloneState &clone_state);

    ResourceWidget<tracer::Texture2D> *clone() override;

    std::unique_ptr<ResourceThumbnailProvider> get_thumbnail(int width, int height) const override;

    void save_asset(AssetSaver &saver) override;

    void load_asset(AssetLoader &loader) override;

    std::shared_ptr<tracer::ConfigNode> to_config(JSONExportContext &ctx) const override;

protected:

    void update_tracer_object_impl() override;

private:

    void do_update_tracer_object();

    void init_ui(const CloneState &clone_state);

    void browse_filename();

    QString filename_;
    std::shared_ptr<const Image2D<math::color3f>> img_data_;

    QVBoxLayout *layout_ = nullptr;

    ElidedLabel *filename_label_   = nullptr;
    QPushButton *filename_button_  = nullptr;
    QPushButton *preview_button_  = nullptr;

    Collapsible                 *adv_section_ = nullptr;
    Texture2DCommonParamsWidget *adv_widget_  = nullptr;
};

class HDRWidgetCreator : public Texture2DWidgetCreator
{
public:

    QString name() const override { return "HDR"; }

    ResourceWidget<tracer::Texture2D> *create_widget(ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
