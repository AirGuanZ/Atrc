#include <QCheckBox>

#include <agz/editor/material/glass.h>
#include <agz/editor/texture2d/range.h>
#include <agz/editor/ui/utility/collapsible.h>

AGZ_EDITOR_BEGIN

GlassWidget::GlassWidget(const InitData &init_data, ObjectContext &obj_ctx)
    : obj_ctx_(obj_ctx)
{
    color_      = init_data.color;
    ior_        = init_data.ior;
    color_refr_ = init_data.color_refr;

    if(!color_)
        color_ = new Texture2DSlot(obj_ctx_, "Constant");

    if(!ior_)
    {
        auto tex = new RangeWidget({ real(1.01), 3, real(1.5) });
        ior_ = new Texture2DSlot(obj_ctx_, "Range", tex, "Range");
    }

    if(!color_refr_)
        color_refr_ = new Texture2DSlot(obj_ctx_, "Constant");

    use_color_refr_ = new QCheckBox("Seperate Refraction Color");
    use_color_refr_->setChecked(init_data.use_color_refr);

    color_refr_->setDisabled(!init_data.use_color_refr);

    Collapsible *color_sec = new Collapsible(this, "Color");
    Collapsible *ior_sec   = new Collapsible(this, "IOR");
    Collapsible *adv_sec   = new Collapsible(this, "Advanced");

    QWidget     *adv_widget = new QWidget;
    QVBoxLayout *adv_layout = new QVBoxLayout(adv_widget);
    adv_layout->addWidget(use_color_refr_);
    adv_layout->addWidget(color_refr_);
    adv_widget->setContentsMargins(0, 0, 0, 0);
    adv_layout->setContentsMargins(0, 0, 0, 0);

    color_sec->set_content_widget(color_);
    ior_sec  ->set_content_widget(ior_);
    adv_sec  ->set_content_widget(adv_widget);

    color_sec->open();
    ior_sec  ->open();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(color_sec);
    layout->addWidget(ior_sec);
    layout->addWidget(adv_sec);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    color_     ->set_dirty_callback([=] { set_dirty_flag(); });
    ior_       ->set_dirty_callback([=] { set_dirty_flag(); });
    color_refr_->set_dirty_callback([=] { set_dirty_flag(); });

    connect(use_color_refr_, &QCheckBox::stateChanged, [=](int)
    {
        color_refr_->setDisabled(!use_color_refr_->isChecked());
        set_dirty_flag();
    });

    do_update_tracer_object();
}

ResourceWidget<tracer::Material> *GlassWidget::clone()
{
    InitData init_data;
    init_data.color          = color_->clone();
    init_data.ior            = ior_  ->clone();
    init_data.color_refr     = color_refr_->clone();
    init_data.use_color_refr = use_color_refr_->isChecked();
    return new GlassWidget(init_data, obj_ctx_);
}

QPixmap GlassWidget::get_thumbnail(int width, int height) const
{
    return QPixmap(width, height);
}

void GlassWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void GlassWidget::do_update_tracer_object()
{
    auto color      = color_->get_tracer_object();
    auto ior        = ior_->get_tracer_object();

    auto color_refr = color;
    if(use_color_refr_->isChecked())
        color_refr = color_refr_->get_tracer_object();

    tracer_object_ = create_glass(color, color_refr, ior);
}

ResourceWidget<tracer::Material> *GlassWidgetCreator::create_widget(ObjectContext &obj_ctx) const
{
    return new GlassWidget({}, obj_ctx);
}

AGZ_EDITOR_END
