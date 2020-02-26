#pragma once

#include <agz/editor/material/material.h>

AGZ_EDITOR_BEGIN

class InvisibleSurfaceWidget : public MaterialWidget
{
public:

    InvisibleSurfaceWidget();

    ResourceWidget<tracer::Material> *clone() override;

    std::unique_ptr<ResourceThumbnailProvider> get_thumbnail(int width, int height) const override;

protected:

    void update_tracer_object_impl() override;

private:

    void do_update_tracer_object();
};

class InvisibleSurfaceWidgetCreator : public MaterialWidgetCreator
{
public:

    QString name() const override
    {
        return "Invisible";
    }

    ResourceWidget<tracer::Material> *create_widget(ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
