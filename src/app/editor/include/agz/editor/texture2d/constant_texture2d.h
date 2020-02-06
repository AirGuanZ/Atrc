#pragma once

#include <agz/editor/texture2d/texture2d.h>
#include <agz/editor/ui/utility/color_holder.h>

AGZ_EDITOR_BEGIN

class ConstantTexture2DWidget : public Texture2DWidget
{
    Q_OBJECT

public:

    explicit ConstantTexture2DWidget(const Spectrum &init_color = { 0, 0, 0 })
    {
        layout_ = new QVBoxLayout(this);
        color_holder_ = new ColorHolder(init_color, this);
        layout_->addWidget(color_holder_);

        connect(color_holder_, &ColorHolder::change_color, [=](const Spectrum &)
        {
            set_dirty_flag();
        });
    }

    Texture2DWidget *clone() override
    {
        return new ConstantTexture2DWidget(color_holder_->get_color());
    }

protected:

    void update_tracer_object_impl() override
    {
        tracer_object_ = tracer::create_constant2d_texture({}, color_holder_->get_color());
    }

private:

    QVBoxLayout *layout_;
    ColorHolder *color_holder_;
};

AGZ_EDITOR_END
