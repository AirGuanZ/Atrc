#include <QFileDialog>
#include <QMessageBox>

#include <agz/editor/imexport/json_export_context.h>
#include <agz/editor/texture3d/image3d.h>
#include <agz/factory/utility/texture3d_loader.h>
#include <agz/tracer/create/texture3d.h>

AGZ_EDITOR_BEGIN

Image3DWidget::Image3DWidget(const InitData &init_data)
{
    real_data_   = init_data.real_data;
    spec_data_   = init_data.spec_data;
    uint8_data_  = init_data.uint8_data;
    uint24_data_ = init_data.uint24_data;

    browse_filename_ = new QPushButton("Browse", this);
    filename_ = new ElidedLabel(init_data.filename, this);
    filename_->setToolTip(init_data.filename);

    connect(browse_filename_, &QPushButton::clicked, [=]
    {
        browse_filename();
    });

    sampler_type_ = new QComboBox(this);
    sampler_type_->addItems({ "Linear", "Nearest" });
    sampler_type_->setCurrentText(init_data.sampler_type);

    connect(sampler_type_, &QComboBox::currentTextChanged, [=]
    {
        set_dirty_flag();
    });

    common_ = init_data.common;
    if(!common_)
    {
        common_ = new Texture3DCommonParamsWidget(
            Texture3DCommonParamsWidget::InitData{});
    }

    connect(common_, &Texture3DCommonParamsWidget::change_params, [=]
    {
        set_dirty_flag();
    });

    common_section_ = new Collapsible(this, "Advance");
    common_section_->set_content_widget(common_);

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(browse_filename_, 0, 0, 1, 1);
    layout->addWidget(filename_, 0, 1, 1, 1);
    layout->addWidget(sampler_type_, 1, 0, 1, 2);
    layout->addWidget(common_section_, 2, 0, 1, 2);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    do_update_tracer_object();
}

ResourceWidget<tracer::Texture3D> *Image3DWidget::clone()
{
    InitData init_data;
    init_data.real_data    = real_data_;
    init_data.spec_data    = spec_data_;
    init_data.uint8_data   = uint8_data_;
    init_data.uint24_data  = uint24_data_;
    init_data.filename     = filename_->text();
    init_data.sampler_type = sampler_type_->currentText();
    init_data.common       = common_->clone();
    return new Image3DWidget(init_data);
}

Box<ResourceThumbnailProvider> Image3DWidget::get_thumbnail(
    int width, int height) const
{
    return newBox<EmptyResourceThumbnailProvider>(width, height);
}

namespace
{
    template<typename Texel>
    void save_image3d(RC<const Image3D<Texel>> &img, AssetSaver &saver)
    {
        if(!img)
        {
            saver.write(uint8_t(0));
            return;
        }

        saver.write(uint8_t(1));
        saver.write(uint32_t(img->width()));
        saver.write(uint32_t(img->height()));
        saver.write(uint32_t(img->depth()));

        const uint32_t byte_size = img->size().product() * sizeof(Texel);
        saver.write_raw(img->raw_data(), byte_size);
    }

    template<typename Texel>
    RC<const Image3D<Texel>> load_image3d(AssetLoader &loader)
    {
        const uint8_t has_data = loader.read<uint8_t>();
        if(!has_data)
            return {};

        const uint32_t width  = loader.read<uint32_t>();
        const uint32_t height = loader.read<uint32_t>();
        const uint32_t depth  = loader.read<uint32_t>();

        auto ret = newRC<Image3D<Texel>>(depth, height, width);

        const uint32_t byte_size = ret->size().product() * sizeof(Texel);
        loader.read_raw(ret->raw_data(), byte_size);

        return ret;
    }
}

void Image3DWidget::save_asset(AssetSaver &saver)
{
    save_image3d(real_data_, saver);
    save_image3d(spec_data_, saver);
    save_image3d(uint8_data_, saver);
    save_image3d(uint24_data_, saver);

    saver.write_string(filename_->text());
    saver.write_string(sampler_type_->currentText());
    common_->save_asset(saver);
}

void Image3DWidget::load_asset(AssetLoader &loader)
{
    real_data_   = load_image3d<real>         (loader);
    spec_data_   = load_image3d<Spectrum>     (loader);
    uint8_data_  = load_image3d<uint8_t>      (loader);
    uint24_data_ = load_image3d<math::color3b>(loader);

    const QString filename = loader.read_string();
    filename_->setText(filename);
    filename_->setToolTip(filename);

    sampler_type_->setCurrentText(loader.read_string());

    common_->load_asset(loader);

    do_update_tracer_object();
}

RC<tracer::ConfigNode> Image3DWidget::to_config(
    JSONExportContext &ctx) const
{
    auto grp = newRC<tracer::ConfigGroup>();

    if(!real_data_ && !spec_data_ && !uint8_data_ && !uint24_data_)
    {
        grp->insert_str("type", "constant");
        grp->insert_child(
            "texel", tracer::ConfigArray::from_spectrum(Spectrum(real(0.5))));
        return grp;
    }

    grp->insert_str("type", "image3d");

    auto [ref_filename, filename] = ctx.gen_filename(".image3d");

    std::string format;

    if(real_data_)
    {
        tracer::texture3d_load::save_real_to_binary(
            filename,
            real_data_->size(),
            real_data_->raw_data());
        format = "real";
    }
    else if(spec_data_)
    {
        tracer::texture3d_load::save_spec_to_binary(
            filename,
            spec_data_->size(),
            &spec_data_->raw_data()->r);
        format = "spec";
    }
    else if(uint8_data_)
    {
        tracer::texture3d_load::save_uint8_to_binary(
            filename,
            uint8_data_->size(),
            uint8_data_->raw_data());
        format = "gray8";
    }
    else
    {
        assert(uint24_data_);
        tracer::texture3d_load::save_uint8_to_binary(
            filename,
            uint24_data_->size(),
            &uint24_data_->raw_data()->r);
        format = "rgb8";
    }

    grp->insert_str("format", format);
    grp->insert_str("binary_filename", ref_filename);
    grp->insert_str("sampler", sampler_type_->currentText().toLower().toStdString());


    common_->to_config(*grp);

    return grp;
}

void Image3DWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void Image3DWidget::do_update_tracer_object()
{
    tracer::Texture3DCommonParams common_params = common_->get_tracer_params();

    if(!real_data_ && !spec_data_ && !uint8_data_ && !uint24_data_)
    {
        tracer_object_ = create_constant3d_texture(
            common_params, Spectrum(real(0.5)));
        return;
    }

    const bool use_linear_sampler = sampler_type_->currentText() == "Linear";

    if(real_data_)
    {
        tracer_object_ = create_image3d(
            common_params, real_data_, use_linear_sampler);
    }
    else if(spec_data_)
    {
        tracer_object_ = create_image3d(
            common_params, spec_data_, use_linear_sampler);
    }
    else if(uint8_data_)
    {
        tracer_object_ = create_image3d(
            common_params, uint8_data_, use_linear_sampler);
    }
    else
    {
        assert(uint24_data_);
        tracer_object_ = create_image3d(
            common_params, uint24_data_, use_linear_sampler);
    }
}

void Image3DWidget::browse_filename()
{
    auto clear_img_data = [&]
    {
        real_data_.reset();
        spec_data_.reset();
        uint8_data_.reset();
        uint24_data_.reset();
    };

    try
    {
        bool ok = false;

        const QStringList filetype_list = { "ASCII", "Binary", "Images" };
        const QString filetype = QInputDialog::getItem(
            this, "Type", "Select file type", filetype_list, 0, false, &ok);
        if(!ok)
            return;

        const QStringList format_list = { "Real", "RGBReal", "Byte", "RGBByte" };
        const QString format = QInputDialog::getItem(
            this, "Format", "Select file format", format_list, 0, false, &ok);
        if(!ok)
            return;

        QString new_filename;

        if(format == "Real")
        {
            RC<const Image3D<real>> new_real_data;

            if(filetype == "ASCII")
            {
                new_filename = QFileDialog::getOpenFileName(this);
                if(new_filename.isEmpty())
                    return;

                std::ifstream fin(new_filename.toStdString());
                if(!fin)
                    throw tracer::ObjectConstructionException(
                        "failed to open file: " + new_filename.toStdString());

                new_real_data = tracer::toRC(
                    tracer::texture3d_load::load_real_from_ascii(fin));

                clear_img_data();
                real_data_ = new_real_data;
            }
            else if(filetype == "Binary")
            {
                new_filename = QFileDialog::getOpenFileName(this);
                if(new_filename.isEmpty())
                    return;

                std::ifstream fin(
                    new_filename.toStdString(), std::ios::in | std::ios::binary);
                new_real_data = tracer::toRC(
                    tracer::texture3d_load::load_real_from_binary(fin));

                clear_img_data();
                real_data_ = new_real_data;
            }
            else
            {
                auto filenames = QFileDialog::getOpenFileNames(this);
                if(filenames.isEmpty())
                    return;
                filenames.sort(Qt::CaseSensitive);
                new_filename = filenames[0];

                std::vector<std::string> std_filenames;
                for(auto &f : filenames)
                    std_filenames.push_back(f.toStdString());

                new_real_data = tracer::toRC(tracer::texture3d_load::load_real_from_images(
                    std_filenames.data(), int(std_filenames.size()),
                    tracer::factory::BasicPathMapper()));

                clear_img_data();
                real_data_ = new_real_data;
            }
        }
        else if(format == "RGBReal")
        {
            RC<const Image3D<Spectrum>> new_spec_data;

            if(filetype == "ASCII")
            {
                new_filename = QFileDialog::getOpenFileName(this);
                if(new_filename.isEmpty())
                    return;

                std::ifstream fin(new_filename.toStdString(), std::ios::in);
                new_spec_data = tracer::toRC(
                    tracer::texture3d_load::load_spec_from_ascii(fin));

                clear_img_data();
                spec_data_ = new_spec_data;
            }
            else if(filetype == "Binary")
            {
                new_filename = QFileDialog::getOpenFileName(this);
                if(new_filename.isEmpty())
                    return;

                std::ifstream fin(
                    new_filename.toStdString(), std::ios::in | std::ios::binary);
                new_spec_data = tracer::toRC(
                    tracer::texture3d_load::load_spec_from_binary(fin));

                clear_img_data();
                spec_data_ = new_spec_data;
            }
            else
            {
                auto filenames = QFileDialog::getOpenFileNames(this);
                if(filenames.isEmpty())
                    return;
                filenames.sort(Qt::CaseSensitive);
                new_filename = filenames[0];

                std::vector<std::string> std_filenames;
                for(auto &f : filenames)
                    std_filenames.push_back(f.toStdString());

                new_spec_data = tracer::toRC(tracer::texture3d_load::load_spec_from_images(
                    std_filenames.data(), int(std_filenames.size()),
                    tracer::factory::BasicPathMapper()));

                clear_img_data();
                spec_data_ = new_spec_data;
            }
        }
        else if(format == "Byte")
        {
            RC<const Image3D<uint8_t>> new_uint8_data;

            if(filetype == "ASCII")
            {
                new_filename = QFileDialog::getOpenFileName(this);
                if(new_filename.isEmpty())
                    return;

                std::ifstream fin(new_filename.toStdString(), std::ios::in);
                new_uint8_data = tracer::toRC(
                    tracer::texture3d_load::load_uint8_from_ascii(fin));

                clear_img_data();
                uint8_data_ = new_uint8_data;
            }
            else if(filetype == "Binary")
            {
                new_filename = QFileDialog::getOpenFileName(this);
                if(new_filename.isEmpty())
                    return;

                std::ifstream fin(
                    new_filename.toStdString(), std::ios::in | std::ios::binary);
                new_uint8_data = tracer::toRC(
                    tracer::texture3d_load::load_uint8_from_binary(fin));

                clear_img_data();
                uint8_data_ = new_uint8_data;
            }
            else
            {
                auto filenames = QFileDialog::getOpenFileNames(this);
                if(filenames.isEmpty())
                    return;
                filenames.sort(Qt::CaseSensitive);
                new_filename = filenames[0];

                std::vector<std::string> std_filenames;
                for(auto &f : filenames)
                    std_filenames.push_back(f.toStdString());

                new_uint8_data = tracer::toRC(tracer::texture3d_load::load_uint8_from_images(
                    std_filenames.data(), int(std_filenames.size()),
                    tracer::factory::BasicPathMapper()));

                clear_img_data();
                uint8_data_ = new_uint8_data;
            }
        }
        else
        {
            RC<const Image3D<math::color3b>> new_uint24_data;

            if(filetype == "ASCII")
            {
                new_filename = QFileDialog::getOpenFileName(this);
                if(new_filename.isEmpty())
                    return;

                std::ifstream fin(new_filename.toStdString(), std::ios::in);
                new_uint24_data = tracer::toRC(
                    tracer::texture3d_load::load_uint24_from_ascii(fin));

                clear_img_data();
                uint24_data_ = new_uint24_data;
            }
            else if(filetype == "Binary")
            {
                new_filename = QFileDialog::getOpenFileName(this);
                if(new_filename.isEmpty())
                    return;

                std::ifstream fin(
                    new_filename.toStdString(), std::ios::in | std::ios::binary);
                new_uint24_data = tracer::toRC(
                    tracer::texture3d_load::load_uint24_from_binary(fin));

                clear_img_data();
                uint24_data_ = new_uint24_data;
            }
            else
            {
                auto filenames = QFileDialog::getOpenFileNames(this);
                if(filenames.isEmpty())
                    return;
                filenames.sort(Qt::CaseSensitive);
                new_filename = filenames[0];

                std::vector<std::string> std_filenames;
                for(auto &f : filenames)
                    std_filenames.push_back(f.toStdString());

                new_uint24_data = tracer::toRC(tracer::texture3d_load::load_uint24_from_images(
                    std_filenames.data(), int(std_filenames.size()),
                    tracer::factory::BasicPathMapper()));

                clear_img_data();
                uint24_data_ = new_uint24_data;
            }
        }

        filename_->setText(new_filename);
        filename_->setToolTip(new_filename);
    }
    catch(...)
    {
        QMessageBox::information(this, "Error", "Failed to load rgb vol data");
        return;
    }

    set_dirty_flag();
}

ResourceWidget<tracer::Texture3D> *Image3DWidgetCreator::create_widget(
    ObjectContext &obj_ctx) const
{
    return new Image3DWidget({});
}

AGZ_EDITOR_END
