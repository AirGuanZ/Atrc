#include <QCheckBox>

#include <agz/editor/texture2d/constant2d.h>

AGZ_EDITOR_BEGIN

Constant2DWidget::Constant2DWidget(const InitData &init_data)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    input_color_     = new SpectrumInput(this);
    use_input_color_ = new QCheckBox("Use Input Color", this);
    color_holder_    = new ColorHolder(init_data.color_holder_value, this);

    use_input_color_->setChecked(init_data.use_input_color);
    input_color_->set_value(
        {
            init_data.input_value.r,
            init_data.input_value.g,
            init_data.input_value.b
        });
    input_color_->set_alignment(Qt::AlignCenter);

    layout->addWidget(use_input_color_);
    layout->addWidget(input_color_);
    layout->addWidget(color_holder_);

    if(init_data.use_input_color)
        color_holder_->hide();
    else
        input_color_->hide();

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    connect(use_input_color_, &QCheckBox::stateChanged, [=](int)
    {
        if(use_input_color_->isChecked())
        {
            color_holder_->hide();
            input_color_->show();
        }
        else
        {
            input_color_->hide();
            color_holder_->show();
        }

        set_dirty_flag();
    });

    connect(input_color_, &SpectrumInput::edit_value, [=](const Spectrum&)
    {
        set_dirty_flag();
    });

    connect(color_holder_, &ColorHolder::change_color, [=](const Spectrum &)
    {
        set_dirty_flag();
    });

    if(use_input_color_->isChecked())
        tracer_object_ = tracer::create_constant2d_texture(
            {}, input_color_->get_value());
    else
        tracer_object_ = tracer::create_constant2d_texture(
            {}, color_holder_->get_color());
}

Texture2DWidget *Constant2DWidget::clone()
{
    InitData init_data;
    init_data.use_input_color    = use_input_color_->isChecked();
    init_data.input_value        = input_color_->get_value();
    init_data.color_holder_value = color_holder_->get_color();

    return new Constant2DWidget(init_data);
}

Box<ResourceThumbnailProvider> Constant2DWidget::get_thumbnail(
    int width, int height) const
{
    QImage image(1, 1, QImage::Format::Format_RGB888);
    image.setPixelColor(0, 0, color_holder_->get_qcolor());

    QPixmap pixmap;
    pixmap.convertFromImage(image);

    return newBox<FixedResourceThumbnailProvider>(pixmap.scaled(width, height));
}

void Constant2DWidget::save_asset(AssetSaver &saver)
{
    saver.write(uint8_t(use_input_color_->isChecked() ? 1 : 0));
    saver.write(color_holder_->get_color());
    saver.write(input_color_->get_value());
}

void Constant2DWidget::load_asset(AssetLoader &loader)
{
    use_input_color_->setChecked(loader.read<uint8_t>() != 0);
    color_holder_   ->set_color(loader.read<Spectrum>());
    input_color_    ->set_value(loader.read<Spectrum>());

    if(use_input_color_->isChecked())
        tracer_object_ = tracer::create_constant2d_texture(
            {}, input_color_->get_value());
    else
        tracer_object_ = tracer::create_constant2d_texture(
            {}, color_holder_->get_color());
}

RC<tracer::ConfigNode> Constant2DWidget::to_config(JSONExportContext &ctx) const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "constant");

    if(use_input_color_->isChecked())
        grp->insert_child(
            "texel", tracer::ConfigArray::from_spectrum(input_color_->get_value()));
    else
        grp->insert_child(
            "texel", tracer::ConfigArray::from_spectrum(color_holder_->get_color()));

    return grp;
}

void Constant2DWidget::update_tracer_object_impl()
{
    if(use_input_color_->isChecked())
        tracer_object_ = tracer::create_constant2d_texture(
            {}, input_color_->get_value());
    else
        tracer_object_ = tracer::create_constant2d_texture(
            {}, color_holder_->get_color());
}

ResourceWidget<tracer::Texture2D> *Constant2DCreator::create_widget(
    ObjectContext &obj_ctx) const
{
    return new Constant2DWidget({});
}

AGZ_EDITOR_END
