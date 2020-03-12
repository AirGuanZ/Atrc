#include <fstream>

#include <QFileDialog>
#include <QMessageBox>

#include <agz/editor/displayer/preview_window.h>
#include <agz/editor/imexport/asset_load_dialog.h>
#include <agz/editor/resource/object_context.h>
#include <agz/editor/scene/scene_mgr.h>

AGZ_EDITOR_BEGIN

AssetLoadDialog::AssetLoadDialog(
    SceneManager        *scene_mgr,
    ObjectContext       *obj_ctx,
    EnvirLightSlot      *envir_light,
    GlobalSettingWidget *global_settings,
    PreviewWindow       *preview_window)
    : scene_mgr_      (scene_mgr),
      obj_ctx_        (obj_ctx),
      envir_light_    (envir_light),
      global_settings_(global_settings),
      preview_window_ (preview_window)
{
    init_ui();
}

bool AssetLoadDialog::is_ok_clicked() const noexcept
{
    return is_ok_clicked_;
}

void AssetLoadDialog::init_ui()
{
    load_envir_light_ = new QCheckBox("Load Envir Light", this);
    load_envir_light_->setChecked(true);

    load_global_settings_ = new QCheckBox("Load Global Settings", this);
    load_global_settings_->setChecked(true);

    load_preview_window_ = new QCheckBox("Load Camera", this);
    load_preview_window_->setChecked(true);

    QPushButton *ok     = new QPushButton("Ok",    this);
    QPushButton *cancel = new QPushButton("Cancel", this);

    QGridLayout *layout = new QGridLayout(this);
    int row = 0;

    layout->addWidget(load_envir_light_,       row, 0, 1, 2);
    layout->addWidget(load_global_settings_, ++row, 0, 1, 2);
    layout->addWidget(load_preview_window_,  ++row, 0, 1, 2);

    layout->addWidget(ok,   ++row, 0, 1, 1);
    layout->addWidget(cancel, row, 1, 1, 1);

    connect(ok, &QPushButton::clicked, [=]
    {
        this->ok();
    });

    connect(cancel, &QPushButton::clicked, [=]
    {
        is_ok_clicked_ = false;
        this->close();
    });
}

void AssetLoadDialog::ok()
{
    const QString load_filename = QFileDialog::getOpenFileName(this);
    if(load_filename.isEmpty())
        return;

    AGZ_SCOPE_GUARD({ this->close(); });

    std::ifstream fin(
        load_filename.toStdString(), std::ios::in | std::ios::binary);
    if(!fin)
    {
        QMessageBox::information(
            this, "Error", "Failed to open file: " + load_filename);
        is_ok_clicked_ = false;
        return;
    }
    is_ok_clicked_ = true;

    AssetLoader loader(fin);

    const uint32_t section_count = loader.read<uint32_t>();
    for(uint32_t s = 0; s < section_count; ++s)
    {
        const AssetSectionType sec_type = loader.read<AssetSectionType>();
        switch(sec_type)
        {
        case AssetSectionType::MaterialPool:
            obj_ctx_->pool<tracer::Material>()->load_asset(loader);
            break;
        case AssetSectionType::MediumPool:
            obj_ctx_->pool<tracer::Medium>()->load_asset(loader);
            break;
        case AssetSectionType::GeometryPool:
            obj_ctx_->pool<tracer::Geometry>()->load_asset(loader);
            break;
        case AssetSectionType::Texture2DPool:
            obj_ctx_->pool<tracer::Texture2D>()->load_asset(loader);
            break;
        case AssetSectionType::Texture3DPool:
            obj_ctx_->pool<tracer::Texture3D>()->load_asset(loader);
            break;
        case AssetSectionType::Entities:
            scene_mgr_->load_asset(loader);
            break;
        case AssetSectionType::EnvirLight:
            if(load_envir_light_->isChecked())
                envir_light_->load_asset(loader);
            else
            {
                auto e = envir_light_->clone();
                e->load_asset(loader);
                delete e;
            }
            break;
        case AssetSectionType::GlobalSettings:
            if(load_global_settings_->isChecked())
                global_settings_->load_asset(loader);
            break;
        case AssetSectionType::PreviewWindow:
            if(load_preview_window_->isChecked())
                preview_window_->load_asset(loader);
            break;
        }
    }

    loader.exec_delayed_oprs();
}

AGZ_EDITOR_END
