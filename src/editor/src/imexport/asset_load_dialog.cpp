#include <fstream>

#include <QFileDialog>
#include <QMessageBox>

#include <agz/editor/displayer/preview_window.h>
#include <agz/editor/imexport/asset_load_dialog.h>
#include <agz/editor/imexport/asset_version.h>
#include <agz/editor/post_processor/post_processor_seq.h>
#include <agz/editor/resource/object_context.h>
#include <agz/editor/renderer/renderer_widget.h>
#include <agz/editor/scene/scene_mgr.h>

AGZ_EDITOR_BEGIN

AssetLoadDialog::AssetLoadDialog(
    SceneManager        *scene_mgr,
    ObjectContext       *obj_ctx,
    EnvirLightSlot      *envir_light,
    GlobalSettingWidget *global_settings,
    FilmFilterPanel     *film_filter,
    PostProcessorSeq    *post_processors,
    PreviewWindow       *preview_window,
    RendererPanel       *renderer_panel)
    : scene_mgr_      (scene_mgr),
      obj_ctx_        (obj_ctx),
      envir_light_    (envir_light),
      global_settings_(global_settings),
      film_filter_    (film_filter),
      post_processors_(post_processors),
      preview_window_ (preview_window),
      renderer_panel_ (renderer_panel)
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

    load_film_filter_ = new QCheckBox("Load Film Filter", this);
    load_film_filter_->setChecked(true);

    load_post_processors_ = new QCheckBox("Load Post Processors", this);
    load_post_processors_->setChecked(true);

    load_preview_window_ = new QCheckBox("Load Camera", this);
    load_preview_window_->setChecked(true);

    load_renderer_panel_ = new QCheckBox("Load Renderer", this);
    load_renderer_panel_->setChecked(true);

    QPushButton *ok     = new QPushButton("Ok",    this);
    QPushButton *cancel = new QPushButton("Cancel", this);

    QGridLayout *layout = new QGridLayout(this);
    int row = 0;

    layout->addWidget(load_envir_light_,       row, 0, 1, 2);
    layout->addWidget(load_global_settings_, ++row, 0, 1, 2);
    layout->addWidget(load_film_filter_,     ++row, 0, 1, 2);
    layout->addWidget(load_post_processors_, ++row, 0, 1, 2);
    layout->addWidget(load_preview_window_,  ++row, 0, 1, 2);
    layout->addWidget(load_renderer_panel_,  ++row, 0, 1, 2);

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
    const QString load_filename = QFileDialog::getOpenFileName(
        this, QString(), QString(), "Atrc (*.atrc)");
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

    AssetLoader loader(fin);
    if(versions::V_latest < loader.version())
    {
        QMessageBox::information(
            this, "Error", "Editor.version < Asset.version");
        is_ok_clicked_ = false;
        return;
    }

    is_ok_clicked_ = true;

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
        case AssetSectionType::PostProcessors:
            if(load_post_processors_->isChecked())
                post_processors_->load_asset(loader);
            break;
        case AssetSectionType::PreviewWindow:
            if(load_preview_window_->isChecked())
                preview_window_->load_asset(loader);
            break;
        case AssetSectionType::Renderer:
            if(load_renderer_panel_->isChecked())
                renderer_panel_->load_asset(loader);
            break;
        case AssetSectionType::FilmFilter:
            if(load_film_filter_->isChecked())
                film_filter_->load_asset(loader);
            break;
        }
    }

    loader.exec_delayed_oprs();
}

AGZ_EDITOR_END
