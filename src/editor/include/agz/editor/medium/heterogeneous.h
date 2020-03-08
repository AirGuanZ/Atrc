#pragma once

#include <QSpinBox>

#include <agz/editor/medium/medium.h>
#include <agz/editor/texture3d/texture3d.h>
#include <agz/editor/ui/transform3d_seq_widget.h>

AGZ_EDITOR_BEGIN

class HeterogeneousWidget : public MediumWidget
{
public:

    struct InitData
    {
        Transform3DSeqWidget *transform = nullptr;

        Texture3DSlot *density = nullptr;
        Texture3DSlot *albedo  = nullptr;
        Texture3DSlot *g       = nullptr;

        int max_scattering_count = (std::numeric_limits<int>::max)();
    };

    HeterogeneousWidget(const InitData &init_data, ObjectContext &obj_ctx);

    ResourceWidget<tracer::Medium> *clone() override;

    std::unique_ptr<ResourceThumbnailProvider> get_thumbnail(int width, int height) const override;

    void save_asset(AssetSaver &saver) override;

    void load_asset(AssetLoader &loader) override;

    std::shared_ptr<tracer::ConfigNode> to_config(JSONExportContext &ctx) const override;

protected:

    void update_tracer_object_impl() override;

private:

    void do_update_tracer_object();

    ObjectContext &obj_ctx_;

    Transform3DSeqWidget *transform_ = nullptr;
    
    Texture3DSlot *density_ = nullptr;
    Texture3DSlot *albedo_  = nullptr;
    Texture3DSlot *g_       = nullptr;

    QSpinBox *max_scattering_count_ = nullptr;
};

class HeterogeneousWidgetCreator : public MediumWidgetCreator
{
public:

    QString name() const override
    {
        return "Heterogeneous";
    }

    ResourceWidget<tracer::Medium> *create_widget(ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
