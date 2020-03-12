#include <agz/editor/envir_light/native_sky.h>
#include <agz/editor/ui/utility/validator.h>

AGZ_EDITOR_BEGIN

NativeSkyWidget::NativeSkyWidget(const CloneData &clone_data)
{
    QGridLayout *layout = new QGridLayout(this);
    top_                = new ColorHolder(clone_data.top, this);
    bottom_             = new ColorHolder(clone_data.bottom, this);

    QLabel *top_text    = new QLabel("Top    Color", this);
    QLabel *bottom_text = new QLabel("Bottom Color", this);

    layout->addWidget(top_text,    0, 0); layout->addWidget(top_,     0, 1);
    layout->addWidget(bottom_text, 1, 0); layout->addWidget(bottom_,  1, 1);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    connect(top_, &ColorHolder::change_color, [=]
    {
        set_dirty_flag();
    });

    connect(bottom_, &ColorHolder::change_color, [=]
    {
        set_dirty_flag();
    });

    do_update_tracer_object();
}

ResourceWidget<tracer::EnvirLight> *NativeSkyWidget::clone()
{
    CloneData clone_data;
    clone_data.top    = top_->get_color();
    clone_data.bottom = bottom_->get_color();
    return new NativeSkyWidget(clone_data);
}

void NativeSkyWidget::save_asset(AssetSaver &saver)
{
    saver.write(top_->get_color());
    saver.write(bottom_->get_color());
}

void NativeSkyWidget::load_asset(AssetLoader &loader)
{
    top_->set_color(loader.read<Spectrum>());
    bottom_->set_color(loader.read<Spectrum>());

    do_update_tracer_object();
}

RC<tracer::ConfigNode> NativeSkyWidget::to_config(
    JSONExportContext &ctx) const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "native_sky");
    grp->insert_child(
        "top", tracer::ConfigArray::from_spectrum(top_->get_color()));
    grp->insert_child(
        "bottom", tracer::ConfigArray::from_spectrum(bottom_->get_color()));
    return grp;
}

void NativeSkyWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void NativeSkyWidget::do_update_tracer_object()
{
    tracer_object_ = tracer::create_native_sky(
        top_->get_color(), bottom_->get_color());
}

ResourceWidget<tracer::EnvirLight> *NativeSkyCreator::create_widget(
    ObjectContext &obj_ctx) const
{
    return new NativeSkyWidget({});
}

AGZ_EDITOR_END
