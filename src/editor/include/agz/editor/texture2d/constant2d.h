#pragma once

#include <agz/editor/texture2d/texture2d.h>
#include <agz/editor/ui/utility/color_holder.h>

AGZ_EDITOR_BEGIN

class Constant2DWidget : public Texture2DWidget
{
    Q_OBJECT

public:

    explicit Constant2DWidget(const Spectrum &init_color = Spectrum(real(0.5)));

    Texture2DWidget *clone() override;

    QPixmap get_thumbnail(int width, int height) const override;

protected:

    void update_tracer_object_impl() override;

private:

    QVBoxLayout *layout_;
    ColorHolder *color_holder_;
};

class Constant2DCreator : public Texture2DWidgetCreator
{
public:

    QString name() const override { return "Constant"; }

    ResourceWidget<tracer::Texture2D> *create_widget(ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
