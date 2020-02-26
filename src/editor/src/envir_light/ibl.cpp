#include <QCheckBox>

#include <agz/editor/envir_light/ibl.h>
#include <agz/editor/ui/utility/collapsible.h>
#include <agz/editor/ui/utility/validator.h>

AGZ_EDITOR_BEGIN

IBLWidget::IBLWidget(Texture2DSlot *tex, ObjectContext &obj_ctx)
    : tex_(tex), obj_ctx_(obj_ctx)
{
    if(!tex_)
        tex_ = new Texture2DSlot(obj_ctx, "HDR");

    Collapsible *tex_section = new Collapsible(this, "Texture");
    tex_section->set_content_widget(tex_);
    tex_section->open();

    importance_sampling_ = new QCheckBox("Use Importance Sampling", this);

    connect(importance_sampling_, &QCheckBox::stateChanged, [=](int)
    {
        set_dirty_flag();
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tex_section);
    layout->addWidget(importance_sampling_);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    tex_->set_dirty_callback([=]
    {
        set_dirty_flag();
    });

    do_update_tracer_object();
}

ResourceWidget<tracer::EnvirLight> *IBLWidget::clone()
{
    return new IBLWidget(tex_->clone(), obj_ctx_);
}

void IBLWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void IBLWidget::do_update_tracer_object()
{
    const bool use_importance_sampling = importance_sampling_->isChecked();
    tracer_object_ = create_ibl_light(tex_->get_tracer_object(), !use_importance_sampling);
}

ResourceWidget<tracer::EnvirLight> *IBLCreator::create_widget(ObjectContext &obj_ctx) const
{
    return new IBLWidget(nullptr, obj_ctx);
}

AGZ_EDITOR_END
