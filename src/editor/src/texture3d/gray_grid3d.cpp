#include <fstream>

#include <QFileDialog>
#include <QMessageBox>

#include <agz/editor/imexport/json_export_context.h>
#include <agz/editor/texture3d/gray_grid3d.h>
#include <agz/factory/factory.h>

AGZ_EDITOR_BEGIN

GrayGrid3DWidget::GrayGrid3DWidget(const InitData &clone_state)
{
    init_ui(clone_state);

    connect(filename_button_, &QPushButton::clicked,
            [=] { browse_filename(); });

    connect(adv_widget_, &Texture3DCommonParamsWidget::change_params,
            [=] { set_dirty_flag(); });

    do_update_tracer_object();
}

ResourceWidget<tracer::Texture3D> *GrayGrid3DWidget::clone()
{
    InitData clone_state;
    clone_state.img_data = img_data_;
    clone_state.adv      = adv_widget_->clone();
    return new GrayGrid3DWidget(clone_state);
}

void GrayGrid3DWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void GrayGrid3DWidget::do_update_tracer_object()
{
    tracer::Texture3DCommonParams common_params = adv_widget_->get_tracer_params();
    
    if(!img_data_)
    {
        tracer_object_ = create_constant3d_texture(
            common_params, Spectrum(real(0.5)));
        return;
    }

    tracer_object_ = create_gray_grid_point3d(common_params, img_data_);
}

void GrayGrid3DWidget::init_ui(const InitData &clone_state)
{
    // filename and image data

    QWidget     *filename_widget  = new QWidget(this);
    QHBoxLayout *filename_hlayout = new QHBoxLayout(filename_widget);
    filename_button_              = new QPushButton("Browse", filename_widget);
    filename_hlayout->addWidget(filename_button_);

    filename_widget ->setContentsMargins(0, 0, 0, 0);
    filename_hlayout->setContentsMargins(0, 0, 0, 0);

    img_data_ = clone_state.img_data;

    // transform

    adv_section_ = new Collapsible(this, "Advanced");

    adv_widget_ = clone_state.adv;
    if(!adv_widget_)
        adv_widget_ = new Texture3DCommonParamsWidget;
    adv_section_->set_content_widget(adv_widget_);

    // layout

    layout_ = new QVBoxLayout(this);
    layout_->addWidget(filename_widget);
    layout_->addWidget(adv_section_);

    setContentsMargins(0, 0, 0, 0);
    layout_->setContentsMargins(0, 0, 0, 0);
}

Box<ResourceThumbnailProvider> GrayGrid3DWidget::get_thumbnail(
    int width, int height) const
{
    // TODO
    return newBox<EmptyResourceThumbnailProvider>(width, height);
}

void GrayGrid3DWidget::save_asset(AssetSaver &saver)
{
    if(!img_data_)
    {
        saver.write(uint8_t(0));
        return;
    }

    saver.write(uint8_t(1));
    saver.write(uint32_t(img_data_->width()));
    saver.write(uint32_t(img_data_->height()));
    saver.write(uint32_t(img_data_->depth()));

    const uint32_t byte_size = img_data_->size().product() * sizeof(real);
    saver.write_raw(img_data_->raw_data(), byte_size);

    adv_widget_->save_asset(saver);
}

void GrayGrid3DWidget::load_asset(AssetLoader &loader)
{
    const bool has_data = loader.read<uint8_t>() != 0;
    if(!has_data)
    {
        img_data_.reset();
        do_update_tracer_object();
        return;
    }

    const uint32_t width  = loader.read<uint32_t>();
    const uint32_t height = loader.read<uint32_t>();
    const uint32_t depth  = loader.read<uint32_t>();

    const uint32_t byte_size = width * height * depth * sizeof(real);
    std::vector<unsigned char> img_data(byte_size);
    loader.read_raw(img_data.data(), img_data.size());

    img_data_ = newRC<Image3D<real>>(
        depth, height, width, reinterpret_cast<const real *>(img_data.data()));

    adv_widget_->load_asset(loader);

    do_update_tracer_object();
}

RC<tracer::ConfigNode> GrayGrid3DWidget::to_config(JSONExportContext &ctx) const
{
    auto grp = newRC<tracer::ConfigGroup>();

    if(!img_data_)
    {
        grp->insert_str("type", "constant");
        grp->insert_child(
            "texel", tracer::ConfigArray::from_spectrum(Spectrum(real(0.5))));
        return grp;
    }

    grp->insert_str("type", "gray_grid");

    auto [ref_filename, filename] = ctx.gen_filename(".gray_grid");

    tracer::texture3d_load::save_gray_to_binary(
        filename,
        { img_data_->width(), img_data_->height(), img_data_->depth() },
        img_data_->raw_data());

    grp->insert_str("binary_filename", ref_filename);

    adv_widget_->to_config(*grp);

    return grp;
}

void GrayGrid3DWidget::browse_filename()
{
    try
    {
        const QStringList item_list = { "ASCII", "Binary", "Images" };
        const QString item = QInputDialog::getItem(
            this, "Type", "Select file type", item_list);

        Image3D<real> data;
        if(item == "ASCII")
        {
            const QString filename = QFileDialog::getOpenFileName(this);
            if(filename.isEmpty())
                return;

            std::ifstream fin(filename.toStdString(), std::ios::in);
            data = tracer::texture3d_load::load_gray_from_ascii(fin);
        }
        else if(item == "Binary")
        {
            const QString filename = QFileDialog::getOpenFileName(this);
            if(filename.isEmpty())
                return;

            std::ifstream fin(
                filename.toStdString(), std::ios::in | std::ios::binary);
            data = tracer::texture3d_load::load_gray_from_binary(fin);
        }
        else
        {
            auto filenames = QFileDialog::getOpenFileNames(this);
            if(filenames.isEmpty())
                return;
            filenames.sort(Qt::CaseSensitive);

            std::vector<std::string> std_filenames;
            for(auto &f : filenames)
                std_filenames.push_back(f.toStdString());

            data = tracer::texture3d_load::load_gray_from_images(
                std_filenames.data(), filenames.size(),
                tracer::factory::BasicPathMapper());
        }

        img_data_ = newRC<Image3D<real>>(std::move(data));
    }
    catch(...)
    {
        QMessageBox::information(this, "Error", "Failed to load gray vol data");
        return;
    }

    set_dirty_flag();
}

ResourceWidget<tracer::Texture3D> *GrayGrid3DWidgetCreator::create_widget(
    ObjectContext &obj_ctx) const
{
    return new GrayGrid3DWidget({});
}

AGZ_EDITOR_END
