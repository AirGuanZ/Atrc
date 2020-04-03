#include <fstream>

#include <QFileDialog>
#include <QMessageBox>

#include <agz/editor/imexport/asset_save_dialog.h>
#include <agz/editor/post_processor/post_processor_seq.h>
#include <agz/editor/renderer/renderer_widget.h>
#include <agz/editor/scene/scene_mgr.h>

AGZ_EDITOR_BEGIN

AssetSaveDialog::AssetSaveDialog(
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
      film_filter_(film_filter),
      post_processors_(post_processors),
      preview_window_ (preview_window),
      renderer_panel_ (renderer_panel)
{
    init_ui();
}

void AssetSaveDialog::init_ui()
{
    save_all_pools_      = new QCheckBox("All Pools",      this);
    save_material_pool_  = new QCheckBox("Material Pool",  this);
    save_medium_pool_    = new QCheckBox("Medium Pool",    this);
    save_geometry_pool_  = new QCheckBox("Geometry Pool",  this);
    save_texture2d_pool_ = new QCheckBox("Texture2D Pool", this);
    save_texture3d_pool_ = new QCheckBox("Texture3D Pool", this);

    save_entities_        = new QCheckBox("Entities",    this);
    save_envir_light_     = new QCheckBox("Envir Light", this);
    save_global_settings_ = new QCheckBox("Global Settings", this);
    save_film_filter_     = new QCheckBox("Film Filter", this);
    save_post_processors_ = new QCheckBox("Post Processors", this);
    save_preview_window_  = new QCheckBox("Camera", this);
    save_renderer_panel_  = new QCheckBox("Renderer", this);

    save_all_pools_->setTristate(true);
    save_all_pools_->setCheckState(Qt::Checked);

    save_material_pool_ ->setChecked(true);
    save_medium_pool_   ->setChecked(true);
    save_geometry_pool_ ->setChecked(true);
    save_texture2d_pool_->setChecked(true);
    save_texture3d_pool_->setChecked(true);

    save_entities_       ->setChecked(true);
    save_envir_light_    ->setChecked(true);
    save_global_settings_->setChecked(true);
    save_film_filter_    ->setChecked(true);
    save_post_processors_->setChecked(true);
    save_preview_window_ ->setChecked(true);
    save_renderer_panel_ ->setChecked(true);

    QPushButton *ok     = new QPushButton("Ok",     this);
    QPushButton *cancel = new QPushButton("Cancel", this);

    QGridLayout *layout = new QGridLayout(this);
    int row = 0;

    layout->addWidget(save_all_pools_,         row, 0, 1, 2);
    layout->addWidget(save_material_pool_,   ++row, 0, 1, 2);
    layout->addWidget(save_medium_pool_,     ++row, 0, 1, 2);
    layout->addWidget(save_geometry_pool_,   ++row, 0, 1, 2);
    layout->addWidget(save_texture2d_pool_,  ++row, 0, 1, 2);
    layout->addWidget(save_texture3d_pool_,  ++row, 0, 1, 2);
    layout->addWidget(save_entities_,        ++row, 0, 1, 2);
    layout->addWidget(save_envir_light_,     ++row, 0, 1, 2);
    layout->addWidget(save_global_settings_, ++row, 0, 1, 2);
    layout->addWidget(save_film_filter_,     ++row, 0, 1, 2);
    layout->addWidget(save_post_processors_, ++row, 0, 1, 2);
    layout->addWidget(save_preview_window_,  ++row, 0, 1, 2);
    layout->addWidget(save_renderer_panel_,  ++row, 0, 1, 2);
    
    layout->addWidget(ok,   ++row, 0, 1, 1);
    layout->addWidget(cancel, row, 1, 1, 1);

    auto update_all_pools_state = [=]
    {
        bool all_yes = true, all_no = true;
        auto f = [&](QCheckBox *c)
        {
            const bool is_checked = c->isChecked();
            all_yes &= is_checked;
            all_no  &= !is_checked;
        };

        f(save_material_pool_);
        f(save_medium_pool_);
        f(save_geometry_pool_);
        f(save_texture2d_pool_);
        f(save_texture3d_pool_);

        if(all_yes)
            save_all_pools_->setCheckState(Qt::Checked);
        else if(all_no)
            save_all_pools_->setCheckState(Qt::Unchecked);
        else
            save_all_pools_->setCheckState(Qt::PartiallyChecked);
    };

    connect(save_all_pools_, &QCheckBox::clicked, [=]
    {
        if(save_all_pools_->checkState() == Qt::Checked)
        {
            save_material_pool_ ->setChecked(true);
            save_medium_pool_   ->setChecked(true);
            save_geometry_pool_ ->setChecked(true);
            save_texture2d_pool_->setChecked(true);
            save_texture3d_pool_->setChecked(true);
        }
        else if(save_all_pools_->checkState() == Qt::Unchecked)
        {
            save_material_pool_ ->setChecked(false);
            save_medium_pool_   ->setChecked(false);
            save_geometry_pool_ ->setChecked(false);
            save_texture2d_pool_->setChecked(false);
            save_texture3d_pool_->setChecked(false);
        }
    });

    connect(save_material_pool_,  &QCheckBox::clicked,
            [=] { update_all_pools_state(); });
    connect(save_medium_pool_,    &QCheckBox::clicked,
            [=] { update_all_pools_state(); });
    connect(save_geometry_pool_,  &QCheckBox::clicked,
            [=] { update_all_pools_state(); });
    connect(save_texture2d_pool_, &QCheckBox::clicked,
            [=] { update_all_pools_state(); });
    connect(save_texture3d_pool_, &QCheckBox::clicked,
            [=] { update_all_pools_state(); });

    connect(ok,     &QPushButton::clicked, [=] { this->ok(); });
    connect(cancel, &QPushButton::clicked, [=] { this->close(); });
}

void AssetSaveDialog::ok()
{
    const QString save_filename = QFileDialog::getSaveFileName(
        this, QString(), QString(), "Atrc (*.atrc)");
    if(save_filename.isEmpty())
        return;

    AGZ_SCOPE_GUARD({ this->close(); });

    std::ofstream fout(
        save_filename.toStdString(), std::ios::binary | std::ios::trunc);
    if(!fout)
    {
        QMessageBox::information(
            this, "Error", "Failed to open file: " + save_filename);
        return;
    }
    AssetSaver saver(fout);

    if(save_material_pool_->isChecked())
        saver.enable_rsc_pool<tracer::Material>();
    if(save_medium_pool_->isChecked())
        saver.enable_rsc_pool<tracer::Medium>();
    if(save_geometry_pool_->isChecked())
        saver.enable_rsc_pool<tracer::Geometry>();
    if(save_texture2d_pool_->isChecked())
        saver.enable_rsc_pool<tracer::Texture2D>();
    if(save_texture3d_pool_->isChecked())
        saver.enable_rsc_pool<tracer::Texture3D>();

    uint32_t section_count = 0;
    auto section_cnt_pos = fout.tellp();
    saver.write(section_count);

    if(save_material_pool_->isChecked())
    {
        ++section_count;
        saver.write(AssetSectionType::MaterialPool);
        obj_ctx_->pool<tracer::Material>()->save_asset(saver);
    }

    if(save_medium_pool_->isChecked())
    {
        ++section_count;
        saver.write(AssetSectionType::MediumPool);
        obj_ctx_->pool<tracer::Medium>()->save_asset(saver);
    }

    if(save_geometry_pool_->isChecked())
    {
        ++section_count;
        saver.write(AssetSectionType::GeometryPool);
        obj_ctx_->pool<tracer::Geometry>()->save_asset(saver);
    }

    if(save_texture2d_pool_->isChecked())
    {
        ++section_count;
        saver.write(AssetSectionType::Texture2DPool);
        obj_ctx_->pool<tracer::Texture2D>()->save_asset(saver);
    }

    if(save_texture3d_pool_->isChecked())
    {
        ++section_count;
        saver.write(AssetSectionType::Texture3DPool);
        obj_ctx_->pool<tracer::Texture3D>()->save_asset(saver);
    }

    if(save_entities_->isChecked())
    {
        ++section_count;
        saver.write(AssetSectionType::Entities);
        scene_mgr_->save_asset(saver);
    }

    if(save_envir_light_->isChecked())
    {
        ++section_count;
        saver.write(AssetSectionType::EnvirLight);
        envir_light_->save_asset(saver);
    }

    if(save_global_settings_->isChecked())
    {
        ++section_count;
        saver.write(AssetSectionType::GlobalSettings);
        global_settings_->save_asset(saver);
    }

    if(save_post_processors_->isChecked())
    {
        ++section_count;
        saver.write(AssetSectionType::PostProcessors);
        post_processors_->save_asset(saver);
    }

    if(save_preview_window_->isChecked())
    {
        ++section_count;
        saver.write(AssetSectionType::PreviewWindow);
        preview_window_->save_asset(saver);
    }

    if(save_renderer_panel_->isChecked())
    {
        ++section_count;
        saver.write(AssetSectionType::Renderer);
        renderer_panel_->save_asset(saver);
    }

    if(save_film_filter_->isChecked())
    {
        ++section_count;
        saver.write(AssetSectionType::FilmFilter);
        film_filter_->save_asset(saver);
    }

    fout.seekp(section_cnt_pos, std::ios::beg);
    saver.write(section_count);
}

AGZ_EDITOR_END
