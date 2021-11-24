#include <QGridLayout>

#include <agz/editor/texture2d/solid_image2d.h>
#include <agz/editor/texture3d/texture3d.h>
#include <agz/tracer/create/texture2d.h>
#include <agz/tracer/create/texture3d.h>

AGZ_EDITOR_BEGIN

SolidImage2DWidget::SolidImage2DWidget(
    const InitData &clone_data, ObjectContext &object_context)
    : object_context_(object_context)
{
    tex3d_ = clone_data.tex3d;
    if(!tex3d_)
        tex3d_ = new Texture3DSlot(object_context, "Constant");

    auto tex3d_text = new QLabel("Texture3D", this);

    tex3d_->set_dirty_callback([=]
    {
        set_dirty_flag();
    });

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(tex3d_text, 0, 0);
    layout->addWidget(tex3d_, 0, 1);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    do_update_tracer_object();
}

ResourceWidget<tracer::Texture2D> *SolidImage2DWidget::clone()
{
    InitData init_data;
    init_data.tex3d = tex3d_->clone();
    return new SolidImage2DWidget(init_data, object_context_);
}

Box<ResourceThumbnailProvider> SolidImage2DWidget::get_thumbnail(
    int width, int height) const
{
    return tex3d_->get_thumbnail(width, height);
}

void SolidImage2DWidget::save_asset(AssetSaver &saver)
{
    tex3d_->save_asset(saver);
}

void SolidImage2DWidget::load_asset(AssetLoader &loader)
{
    tex3d_->load_asset(loader);
}

RC<tracer::ConfigNode> SolidImage2DWidget::to_config(
    JSONExportContext &ctx) const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "solid_image");
    grp->insert_child("tex3d", tex3d_->to_config(ctx));
    return grp;
}

void SolidImage2DWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void SolidImage2DWidget::do_update_tracer_object()
{
    auto tex3d = tex3d_->get_tracer_object();
    tracer_object_ = tracer::create_solid_image_texture(std::move(tex3d));
}

ResourceWidget<tracer::Texture2D> *SolidImage2DCreator::create_widget(
    ObjectContext &obj_ctx) const
{
    return new SolidImage2DWidget({}, obj_ctx);
}

AGZ_EDITOR_END
