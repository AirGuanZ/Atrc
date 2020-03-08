#include <QFileDialog>
#include <QMessageBox>

#include <agz/editor/imexport/json_export_context.h>
#include <agz/editor/texture2d/hdr.h>
#include <agz/utility/image.h>
#include <agz/utility/system.h>

AGZ_EDITOR_BEGIN

HDRWidget::HDRWidget(const CloneState &clone_state)
{
    init_ui(clone_state);

    connect(preview_button_, &QPushButton::clicked, [=]
    {
        if(!filename_.isEmpty())
            sys::open_with_default_app(filename_.toStdString());
    });

    connect(filename_button_, &QPushButton::clicked, [=] { browse_filename(); });

    connect(adv_widget_, &Texture2DCommonParamsWidget::change_params, [=] { set_dirty_flag(); });

    do_update_tracer_object();
}

ResourceWidget<tracer::Texture2D> *HDRWidget::clone()
{
    CloneState clone_state;
    clone_state.filename               = filename_;
    clone_state.img_data               = img_data_;
    clone_state.adv                    = adv_widget_->clone();
    return new HDRWidget(clone_state);
}

void HDRWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void HDRWidget::do_update_tracer_object()
{
    tracer::Texture2DCommonParams common_params = adv_widget_->get_tracer_params();

    if(!img_data_)
    {
        tracer_object_ = create_constant2d_texture(common_params, Spectrum(real(0.5)));
        return;
    }

    tracer_object_ = create_hdr_texture(common_params, img_data_, "linear");
}

void HDRWidget::init_ui(const CloneState &clone_state)
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

    // transform

    adv_section_ = new Collapsible(this, "Advanced");

    adv_widget_ = clone_state.adv;
    if(!adv_widget_)
        adv_widget_ = new Texture2DCommonParamsWidget;
    adv_section_->set_content_widget(adv_widget_);

    // layout

    layout_ = new QVBoxLayout(this);
    layout_->addWidget(filename_widget);
    layout_->addWidget(preview_button_);
    layout_->addWidget(adv_section_);

    setContentsMargins(0, 0, 0, 0);
    layout_->setContentsMargins(0, 0, 0, 0);
}

std::unique_ptr<ResourceThumbnailProvider> HDRWidget::get_thumbnail(int width, int height) const
{
    if(!img_data_)
    {
        QImage img(1, 1, QImage::Format::Format_RGB888);
        img.setPixelColor(0, 0, Qt::black);

        QPixmap pixmap;
        pixmap.convertFromImage(img);
        return std::make_unique<FixedResourceThumbnailProvider>(pixmap.scaled(width, height));
    }

    auto imgu8 = img_data_->map(&math::to_color3b<float>);

    const QImage img(
        reinterpret_cast<const uchar *>(&imgu8.raw_data()[0]),
        imgu8.width(), imgu8.height(),
        sizeof(math::color3b) *imgu8.width(),
        QImage::Format::Format_RGB888);

    QPixmap pixmap;
    pixmap.convertFromImage(img);

    return std::make_unique<FixedResourceThumbnailProvider>(pixmap.scaled(width, height));
}

void HDRWidget::save_asset(AssetSaver &saver)
{
    saver.write_string(filename_);

    if(img_data_)
    {
        const auto img_data = img::save_rgb_to_hdr_in_memory(
            img_data_->raw_data(), img_data_->width(), img_data_->height());

        saver.write(uint32_t(img_data.size()));
        saver.write_raw(img_data.data(), img_data.size());
    }
    else
        saver.write(uint32_t(0));

    adv_widget_->save_asset(saver);
}

void HDRWidget::load_asset(AssetLoader &loader)
{
    filename_ = loader.read_string();
    filename_label_->setText(filename_);
    filename_label_->setToolTip(filename_);

    const uint32_t img_data_byte_size = loader.read<uint32_t>();
    if(img_data_byte_size)
    {
        std::vector<unsigned char> img_data(img_data_byte_size);
        loader.read_raw(img_data.data(), img_data_byte_size);

        img_data_ = std::make_shared<Image2D<math::color3f>>(
            img::load_rgb_from_hdr_memory(img_data.data(), img_data.size()));
    }
    else
        img_data_.reset();

    adv_widget_->load_asset(loader);

    do_update_tracer_object();
}

std::shared_ptr<tracer::ConfigNode> HDRWidget::to_config(JSONExportContext &ctx) const
{
    auto grp = std::make_shared<tracer::ConfigGroup>();

    if(!img_data_)
    {
        grp->insert_str("type", "constant");
        grp->insert_child("texel", tracer::ConfigArray::from_spectrum(Spectrum(0.5f)));
        return grp;
    }

    grp->insert_str("type", "hdr");

    auto [ref_filename, filename] = ctx.gen_filename(".hdr");
    img::save_rgb_to_hdr_file(filename, img_data_->raw_data(), img_data_->width(), img_data_->height());

    grp->insert_str("filename", ref_filename);

    adv_widget_->to_config(*grp);

    return grp;
}

void HDRWidget::browse_filename()
{
    const QString filename = QFileDialog::getOpenFileName(this, "Load Image");
    if(filename.isEmpty())
        return;

    try
    {
        auto data = img::load_rgb_from_hdr_file(filename.toStdString());
        if(!data.is_available())
            throw std::runtime_error("failed");

        img_data_ = std::make_shared<Image2D<math::color3f>>(std::move(data));
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

ResourceWidget<tracer::Texture2D> *HDRWidgetCreator::create_widget(ObjectContext &obj_ctx) const
{
    return new HDRWidget({});
}

AGZ_EDITOR_END
