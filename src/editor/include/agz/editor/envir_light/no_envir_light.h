#pragma once

#include <agz/editor/envir_light/envir_light.h>

AGZ_EDITOR_BEGIN

class NoEnvirLightWidget : public EnvirLightWidget
{
public:

    ResourceWidget<tracer::EnvirLight> *clone() override
    {
        return new NoEnvirLightWidget;
    }

protected:

    void update_tracer_object_impl() override
    {
        // do nothing
    }
};

class NoEnvirLightWidgetCreaotr : public EnvirLightWidgetCreator
{
public:

    QString name() const override
    {
        return "No Envir Light";
    }

    ResourceWidget<tracer::EnvirLight> *create_widget(ObjectContext &obj_ctx) const override
    {
        return new NoEnvirLightWidget;
    }
};

AGZ_EDITOR_END
