#pragma once

#include <agz/editor/medium/medium.h>

AGZ_EDITOR_BEGIN

class VoidWidget : public MediumWidget
{
public:

    VoidWidget();

    ResourceWidget<tracer::Medium> *clone() override;

    void save_asset(AssetSaver &saver) override { }

    void load_asset(AssetLoader &loader) override { }

    std::shared_ptr<tracer::ConfigNode> to_config(JSONExportContext &ctx) const override;

protected:

    void update_tracer_object_impl() override;
};

class VoidWidgetCreator : public MediumWidgetCreator
{
public:

    QString name() const override
    {
        return "Void";
    }

    ResourceWidget<tracer::Medium> *create_widget(ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
