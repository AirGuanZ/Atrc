#include <QFileDialog>
#include <QMessageBox>

#include <agz/editor/imexport/json_export_context.h>
#include <agz/editor/texture2d/image2d.h>
#include <agz/tracer/create/texture2d.h>
#include <agz-utils/image.h>
#include <agz-utils/system.h>

AGZ_EDITOR_BEGIN

Image2DWidget::Image2DWidget(const InitData &clone_state)
{
    init_ui(clone_state);

    connect(preview_button_, &QPushButton::clicked, [=]
    {
        if(!filename_.isEmpty())
            sys::open_with_default_app(filename_.toStdString());
    });

    connect(filename_button_, &QPushButton::clicked, [=] { browse_filename(); });

    connect(sample_method_, &QComboBox::currentTextChanged,
        [=](const QString &new_sample_method)
    {
        set_dirty_flag();
    });

    connect(adv_widget_, &Texture2DCommonParamsWidget::change_params,
        [=] { set_dirty_flag(); });

    do_update_tracer_object();
}

ResourceWidget<tracer::Texture2D> *Image2DWidget::clone()
{
    InitData clone_state;
    clone_state.filename               = filename_;
    clone_state.img_data               = img_data_;
    clone_state.sample_method          = sample_method_->currentText();
    clone_state.adv                    = adv_widget_->clone();
    return new Image2DWidget(clone_state);
}

Box<ResourceThumbnailProvider> Image2DWidget::get_thumbnail(
    int width, int height) const
{
    if(!img_data_)
    {
        QImage img(1, 1, QImage::Format::Format_RGB888);
        img.setPixelColor(0, 0, Qt::black);

        QPixmap pixmap;
        pixmap.convertFromImage(img);
        return newBox<FixedResourceThumbnailProvider>(
            pixmap.scaled(width, height));
    }

    const QImage img(
        reinterpret_cast<const uchar *>(&img_data_->raw_data()[0]),
        img_data_->width(), img_data_->height(),
        sizeof(math::color3b) * img_data_->width(),
        QImage::Format::Format_RGB888);

    QPixmap pixmap;
    pixmap.convertFromImage(img);

    return newBox<FixedResourceThumbnailProvider>(
        pixmap.scaled(width, height));
}

void Image2DWidget::save_asset(AssetSaver &saver)
{
    saver.write_string(filename_);
    saver.write_string(sample_method_->currentText());

    if(img_data_)
    {
        const auto img_data = img::save_rgb_to_png_in_memory(
            img_data_->raw_data(), img_data_->width(), img_data_->height());
        saver.write(uint32_t(img_data.size()));
        saver.write_raw(img_data.data(), img_data.size());
    }
    else
        saver.write(uint32_t(0));

    adv_widget_->save_asset(saver);
}

void Image2DWidget::load_asset(AssetLoader &loader)
{
    filename_ = loader.read_string();
    filename_label_->setText(filename_);
    filename_label_->setToolTip(filename_);

    sample_method_->setCurrentText(loader.read_string());

    const uint32_t img_data_byte_size = loader.read<uint32_t>();

    if(img_data_byte_size)
    {
        std::vector<unsigned char> img_data(img_data_byte_size);
        loader.read_raw(img_data.data(), img_data_byte_size);
        img_data_ = newRC<Image2D<math::color3b>>(
            img::load_rgb_from_memory(img_data.data(), img_data.size()));
    }
    else
        img_data_.reset();

    adv_widget_->load_asset(loader);

    do_update_tracer_object();
}

RC<tracer::ConfigNode> Image2DWidget::to_config(JSONExportContext &ctx) const
{
    auto grp = newRC<tracer::ConfigGroup>();

    if(!img_data_)
    {
        grp->insert_str("type", "constant");
        grp->insert_child("texel", tracer::ConfigArray::from_spectrum(Spectrum(0.5f)));
        return grp;
    }

    grp->insert_str("type", "image");

    auto [ref_filename, filename] = ctx.gen_filename(".png");
    img::save_rgb_to_png_file(
        filename, img_data_->raw_data(), img_data_->width(), img_data_->height());

    grp->insert_str("filename", ref_filename);
    grp->insert_str(
        "sample", sample_method_->currentText().toLower().toStdString());

    adv_widget_->to_config(*grp);

    return grp;
}

void Image2DWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void Image2DWidget::do_update_tracer_object()
{
    tracer::Texture2DCommonParams common_params = adv_widget_->get_tracer_params();
    
    if(!img_data_)
    {
        tracer_object_ = create_constant2d_texture(
            common_params, Spectrum(real(0.5)));
        return;
    }

    tracer_object_ = create_image_texture(
        common_params, img_data_,
        sample_method_->currentText().toLower().toStdString());
}

void Image2DWidget::init_ui(const InitData &clone_state)
{
    // filename and image data

    QWidget     *filename_widget  = new QWidget(this);
    QHBoxLayout *filename_hlayout = new QHBoxLayout(filename_widget);
    filename_button_              = new QPushButton("Browse", filename_widget);
    filename_label_               = new ElidedLabel("", filename_widget);
    filename_hlayout->addWidget(filename_button_);
    filename_hlayout->addWidget(filename_label_);

    filename_widget ->setContentsMargins(0, 0, 0, 0);
    filename_hlayout->setContentsMargins(0, 0, 0, 0);

    img_data_ = clone_state.img_data;
    filename_ = clone_state.filename;
    filename_label_->setText(clone_state.filename);
    filename_label_->setToolTip(clone_state.filename);

    // previewer

    preview_button_ = new QPushButton("Preview");

    // sample

    sample_method_ = new QComboBox(this);
    sample_method_->addItems({ "Linear", "Nearest" });
    sample_method_->setCurrentText(clone_state.sample_method);

    // transform

    adv_section_ = new Collapsible(this, "Advanced");

    adv_widget_ = clone_state.adv;
    if(!adv_widget_)
        adv_widget_ = new Texture2DCommonParamsWidget;
    adv_section_->set_content_widget(adv_widget_);

    // layout

    layout_ = new QVBoxLayout(this);
    layout_->addWidget(filename_widget);
    layout_->addWidget(sample_method_);
    layout_->addWidget(preview_button_);
    layout_->addWidget(adv_section_);

    setContentsMargins(0, 0, 0, 0);
    layout_->setContentsMargins(0, 0, 0, 0);
}

void Image2DWidget::browse_filename()
{
    const QString filename = QFileDialog::getOpenFileName(this, "Load Image");
    if(filename.isEmpty())
        return;

    try
    {
        auto data = img::load_rgb_from_file(filename.toStdString());
        if(!data.is_available())
            throw std::runtime_error("failed");

        img_data_ = newRC<Image2D<math::color3b>>(std::move(data));
    }
    catch(...)
    {
        QMessageBox mbox;
        mbox.setWindowTitle("Error");
        mbox.setText("Failed to load image from " + filename);
        mbox.exec();
        return;
    }

    filename_ = filename;
    filename_label_->setText(filename);
    filename_label_->setToolTip(filename);

    set_dirty_flag();
}

ResourceWidget<tracer::Texture2D> *Image2DCreator::create_widget(
    ObjectContext &obj_ctx) const
{
    return new Image2DWidget({});
}

AGZ_EDITOR_END
