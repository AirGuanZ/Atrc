#include <QFileDialog>

#include <agz/editor/texture2d/image2d.h>
#include <agz/utility/image.h>
#include <agz/utility/system.h>

AGZ_EDITOR_BEGIN

Image2DWidget::Image2DWidget(const CloneState &clone_state)
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

ResourceWidget<tracer::Texture2D> *Image2DWidget::clone()
{
    CloneState clone_state;
    clone_state.filename               = filename_;
    clone_state.img_data               = img_data_;
    clone_state.adv                    = adv_widget_->clone();
    return new Image2DWidget(clone_state);
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
        tracer_object_ = create_constant2d_texture(common_params, Spectrum(real(0.5)));
        return;
    }

    tracer_object_ = create_image_texture(common_params, img_data_, "linear");
}

void Image2DWidget::init_ui(const CloneState &clone_state)
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

QPixmap Image2DWidget::get_thumbnail(int width, int height) const
{
    if(!img_data_)
    {
        QImage img(1, 1, QImage::Format::Format_RGB888);
        img.setPixelColor(0, 0, Qt::black);

        QPixmap pixmap;
        pixmap.convertFromImage(img);
        return pixmap.scaled(width, height);
    }

    const QImage img(
        reinterpret_cast<const uchar *>(&img_data_->raw_data()[0]),
        img_data_->width(), img_data_->height(),
        sizeof(math::color3b) * img_data_->width(),
        QImage::Format::Format_RGB888);

    QPixmap pixmap;
    pixmap.convertFromImage(img);

    return pixmap.scaled(width, height);
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

        img_data_ = std::make_shared<Image2D<math::color3b>>(std::move(data));
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

ResourceWidget<tracer::Texture2D> *Image2DCreator::create_widget(ObjectContext &obj_ctx) const
{
    return new Image2DWidget({});
}

AGZ_EDITOR_END
