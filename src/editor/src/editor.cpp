#include <QFileDialog>
#include <QMenuBar>
#include <QWidgetAction>

#include <agz/editor/editor.h>
#include <agz/editor/imexport/asset_load_dialog.h>
#include <agz/editor/imexport/asset_save_dialog.h>
#include <agz/editor/imexport/json_export.h>
#include <agz/editor/renderer/renderer_widget.h>
#include <agz/editor/ui/global_setting_widget.h>
#include <agz/tracer/utility/embree.h>
#include <agz/tracer/utility/logger.h>

#include <agz-utils/file.h>

AGZ_EDITOR_BEGIN

Editor::Editor()
{
    init_panels();

    AGZ_INFO(">>> Atrc Scene Editor by AirGuanZ <<<");
    
#ifdef USE_EMBREE
    AGZ_INFO("Embree is enabled");
    tracer::init_embree_device();
#endif

#ifdef USE_OIDN
    AGZ_INFO("OIDN is enabled");
#endif

    AGZ_INFO("initialize displayer");

    init_displayer();

    AGZ_INFO("initialize object context");

    init_obj_context();

    AGZ_INFO("initialize scene manager");

    init_scene_mgr();

    AGZ_INFO("initialize renderer panel");

    init_renderer_panel();

    AGZ_INFO("initialize asset saving dialog");

    init_save_asset_dialog();

    AGZ_INFO("initialize render menu");

    init_render_menu();

    AGZ_INFO("initialize global settings");

    init_global_setting_widget();

    AGZ_INFO("initialize post processor editor");

    init_post_processor_widget();

    AGZ_INFO("initialize film filter editor");

    init_film_filter();

    redistribute_panels();

    AGZ_INFO("initialize initial scene");

    scene_params_.envir_light = envir_light_slot_->get_tracer_object();
    scene_params_.aggregate = scene_mgr_->update_tracer_aggregate(
        scene_params_.entities);
    scene_ = create_default_scene(scene_params_);
    scene_->set_camera(preview_window_->create_camera());

    AGZ_INFO("launch renderer");

    launch_renderer(true);
}

Editor::~Editor()
{
    scene_params_ = tracer::DefaultSceneParams();

    renderer_ .reset();
    scene_    .reset();
    scene_mgr_.reset();

    obj_ctx_.reset();

#ifdef USE_EMBREE
    AGZ_SCOPE_GUARD({
        agz::tracer::destroy_embree_device();
    });
#endif
}

void Editor::on_change_camera()
{
    if(!scene_)
        return;

    std::thread destroy_renderer_thread([renderer = std::move(renderer_)] { });
    destroy_renderer_thread.detach();
    
    scene_ = create_default_scene(scene_params_);
    auto camera = preview_window_->create_camera();
    scene_->set_camera(camera);

    launch_renderer(true);
}

void Editor::on_change_aggregate()
{
    if(!is_aggregate_dirty_)
        return;
    is_aggregate_dirty_ = false;

    auto old_camera = scene_->get_shared_camera();

    renderer_.reset();
    scene_params_.entities.clear();
    scene_params_.aggregate = scene_mgr_->update_tracer_aggregate(
        scene_params_.entities);
    scene_ = create_default_scene(scene_params_);
    scene_->set_camera(old_camera);

    launch_renderer(true);
}

void Editor::add_to_resource_panel(QWidget *widget)
{
    right_panel_->resource_tab_layout->addWidget(widget);
    widget->hide();
}

void Editor::show_resource_panel(QWidget *widget, bool display_rsc_panel)
{
    if(editing_rsc_widget_)
        editing_rsc_widget_->hide();

    widget->show();
    editing_rsc_widget_ = widget;

    if(display_rsc_panel)
    {
        right_panel_->right_tab_widget->setCurrentWidget(
            right_panel_->resource_area);
    }
}

void Editor::add_to_entity_panel(QWidget *widget)
{
    right_panel_->entity_tab_layout->addWidget(widget);
    widget->hide();
}

void Editor::show_entity_panel(QWidget *widget, bool display_entity_panel)
{
    if(editing_entity_widget_)
        editing_entity_widget_->hide();

    widget->show();
    editing_entity_widget_ = widget;

    if(display_entity_panel)
    {
        right_panel_->right_tab_widget->setCurrentWidget(
            right_panel_->entity_area);
    }
}

void Editor::on_update_display()
{
    if(renderer_)
    {
        const auto img = renderer_->get_image();
        if(img.is_available())
            set_display_image(img);
    }
}

void Editor::on_change_renderer()
{
    if(!scene_)
        return;

    renderer_.reset();

    launch_renderer(true);
}

void Editor::on_update_envir_light()
{
    if(!scene_)
        return;

    renderer_.reset();

    scene_params_.envir_light = envir_light_slot_->get_tracer_object();

    auto old_camera = scene_->get_shared_camera();

    scene_ = create_default_scene(scene_params_);
    scene_->set_camera(old_camera);

    launch_renderer(true);
}

void Editor::init_panels()
{
    hori_splitter_ = new QSplitter(this);
    setCentralWidget(hori_splitter_);

    left_panel_    = new LeftPanel(hori_splitter_);
    vert_splitter_ = new QSplitter(hori_splitter_);
    right_panel_   = new RightPanel(hori_splitter_);

    hori_splitter_->setOrientation(Qt::Horizontal);
    hori_splitter_->addWidget(left_panel_);
    hori_splitter_->addWidget(vert_splitter_);
    hori_splitter_->addWidget(right_panel_);

    up_panel_   = new QFrame(vert_splitter_);
    down_panel_ = new DownPanel(vert_splitter_);

    up_panel_->setFrameShape(QFrame::Box);

    vert_splitter_->setOrientation(Qt::Vertical);
    vert_splitter_->addWidget(up_panel_);
    vert_splitter_->addWidget(down_panel_);
}

void Editor::init_displayer()
{
    preview_window_ = new PreviewWindow(up_panel_, this);
    
    QVBoxLayout *up_panel_layout = new QVBoxLayout(up_panel_);
    up_panel_layout->addWidget(preview_window_);

    connect(preview_window_, &PreviewWindow::edit_render_mode, [=]
    {
        renderer_.reset();
        launch_renderer(true);
    });

    update_display_timer_ = new QTimer(this);
    update_display_timer_->setInterval(200);
    connect(update_display_timer_, &QTimer::timeout,
            [=] { on_update_display(); });
    update_display_timer_->start();
}

void Editor::init_obj_context()
{
    obj_ctx_ = newBox<ObjectContext>(this);

    down_panel_->material_tab_layout->addWidget(
        obj_ctx_->pool<tracer::Material>()->get_widget());

    down_panel_->medium_tab_layout->addWidget(
        obj_ctx_->pool<tracer::Medium>()->get_widget());

    down_panel_->texture2d_tab_layout->addWidget(
        obj_ctx_->pool<tracer::Texture2D>()->get_widget());

    down_panel_->texture3d_tab_layout->addWidget(
        obj_ctx_->pool<tracer::Texture3D>()->get_widget());

    down_panel_->geometry_tab_layout->addWidget(
        obj_ctx_->pool<tracer::Geometry>()->get_widget());
}

void Editor::init_scene_mgr()
{
    scene_mgr_ = newBox<SceneManager>(*obj_ctx_, this, preview_window_);
    left_panel_->up_panel_layout->addWidget(scene_mgr_->get_widget());

    connect(scene_mgr_.get(), &SceneManager::change_scene, [=]
    {
        is_aggregate_dirty_ = true;
        emit change_aggregate();
        //on_change_aggregate();
    });

    connect(this, &Editor::change_aggregate,
            this, &Editor::on_change_aggregate, Qt::QueuedConnection);
}

void Editor::init_renderer_panel()
{
    // renderer

    renderer_panel_ = new RendererPanel(
        right_panel_->renderer_tab, "Path Tracer");
    right_panel_->renderer_tab_layout->addWidget(renderer_panel_);

    connect(renderer_panel_, &RendererPanel::change_renderer_params,
            [=] { on_change_renderer(); });
    connect(renderer_panel_, &RendererPanel::change_renderer_type,
            [=] { on_change_renderer(); });

    // camera

    right_panel_->camera_tab_layout->addWidget(
        preview_window_->get_camera_panel());

    // environment light

    envir_light_slot_ = new EnvirLightSlot(*obj_ctx_, "Native Sky");
    envir_light_slot_->set_dirty_callback([=]
    {
        emit change_envir_light();
    });
    connect(this, &Editor::change_envir_light, [=] { on_update_envir_light(); });

    right_panel_->envir_light_tab_layout->addWidget(envir_light_slot_);
}

void Editor::init_global_setting_widget()
{
    QWidgetAction *action = new QWidgetAction(this);
    global_setting_ = new GlobalSettingWidget(this);
    action->setDefaultWidget(global_setting_);
    menuBar()->addMenu("Global Settings")->addAction(action);

    global_setting_->scene_eps->setValue(tracer::EPS());
    connect(global_setting_->scene_eps,
            qOverload<double>(&QDoubleSpinBox::valueChanged),
            [=](double new_eps)
    {
        renderer_.reset();
        tracer::set_eps(new_eps);
        launch_renderer(true);
    });
}

void Editor::init_post_processor_widget()
{
    QAction *action = new QAction("Post Processors", this);
    menuBar()->addAction(action);

    pp_seq_ = newBox<PostProcessorSeq>(nullptr);
    connect(action, &QAction::triggered, [=]
    {
        pp_seq_->exec();
    });
}

void Editor::init_film_filter()
{
    QAction *action = new QAction("Film Filter", this);
    menuBar()->addAction(action);

    film_filter_ = newBox<FilmFilterPanel>(this);
    connect(action, &QAction::triggered, [=]
    {
        film_filter_->exec();
    });
}

void Editor::init_save_asset_dialog()
{
    QAction *load_action = new QAction("Load", this);
    menuBar()->addAction(load_action);
    connect(load_action, &QAction::triggered, [=]
    {
        if(!asset_load_dialog_)
        {
            asset_load_dialog_ = newBox<AssetLoadDialog>(
                scene_mgr_.get(), obj_ctx_.get(),
                envir_light_slot_, global_setting_,
                film_filter_.get(), pp_seq_.get(),
                preview_window_, renderer_panel_);
        }

        asset_load_dialog_->exec();
        if(!asset_load_dialog_->is_ok_clicked())
            return;

        tracer::set_eps(real(global_setting_->scene_eps->value()));

        renderer_.reset();

        scene_params_.envir_light = envir_light_slot_->get_tracer_object();
        scene_params_.aggregate = scene_mgr_->update_tracer_aggregate(
            scene_params_.entities);

        scene_ = create_default_scene(scene_params_);
        scene_->set_camera(preview_window_->create_camera());

        launch_renderer(true);
    });

    QAction *save_action = new QAction("Save", this);
    menuBar()->addAction(save_action);
    connect(save_action, &QAction::triggered, [=]
    {
        if(!asset_save_dialog_)
        {
            asset_save_dialog_ = newBox<AssetSaveDialog>(
                scene_mgr_.get(), obj_ctx_.get(),
                envir_light_slot_, global_setting_,
                film_filter_.get(), pp_seq_.get(),
                preview_window_, renderer_panel_);
        }
        asset_save_dialog_->exec();
    });

    QAction *export_json_action = new QAction("Export JSON", this);
    menuBar()->addAction(export_json_action);
    connect(export_json_action, &QAction::triggered, [=]
    {
        export_json(
            scene_mgr_.get(),
            obj_ctx_.get(),
            envir_light_slot_,
            pp_seq_.get(),
            preview_window_,
            global_setting_,
            film_filter_.get(),
            renderer_panel_);
    });
}

void Editor::init_render_menu()
{
    QAction *render_scene = new QAction("Render", this);
    menuBar()->addAction(render_scene);
    connect(render_scene, &QAction::triggered, [=]
    {
        if(!std::filesystem::exists("./AtrcEditorOutput"))
            std::filesystem::create_directory("./AtrcEditorOutput");

        if(!export_json_to_file(
            "./AtrcEditorOutput/scene.json",
            scene_mgr_.get(), obj_ctx_.get(), envir_light_slot_,
            pp_seq_.get(), preview_window_, global_setting_, film_filter_.get(),
            renderer_panel_))
            return;

        if(!gui_render_window_)
            gui_render_window_ = newBox<GUI>();

        gui_render_window_->setWindowTitle("Atrc Renderer");
        gui_render_window_->resize(640, 480);

        preview_window_->set_realtime_mode(true);

        gui_render_window_->showMaximized();
        gui_render_window_->load_config("./AtrcEditorOutput/scene.json");
    });

    QAction *render_scene_to = new QAction("Render To", this);
    menuBar()->addAction(render_scene_to);
    connect(render_scene_to, &QAction::triggered, [=]
    {
        if(!std::filesystem::exists("./AtrcEditorOutput"))
            std::filesystem::create_directory("./AtrcEditorOutput");

        const QString scene_desc_filename = QFileDialog::getSaveFileName(
            this, "Output Directory",
            "./AtrcEditorOutput/scene.json", "JSON (*.json)");

        if(scene_desc_filename.isEmpty())
            return;

        if(!export_json_to_file(
            scene_desc_filename.toStdString(),
            scene_mgr_.get(), obj_ctx_.get(), envir_light_slot_,
            pp_seq_.get(), preview_window_, global_setting_, film_filter_.get(),
            renderer_panel_))
            return;

        if(!gui_render_window_)
            gui_render_window_ = newBox<GUI>();

        gui_render_window_->setWindowTitle(
            "Atrc Renderer: " + scene_desc_filename);
        gui_render_window_->resize(640, 480);

        preview_window_->set_realtime_mode(true);

        gui_render_window_->showMaximized();
        gui_render_window_->load_config(scene_desc_filename.toStdString());
    });
}

void Editor::redistribute_panels()
{
    const int ver_init_height = (std::max)(
        preview_window_->minimumSizeHint().height(),
        down_panel_->minimumSizeHint().height());
    vert_splitter_->setSizes(QList<int>(
        { 2 * ver_init_height, ver_init_height }));

    const int hor_init_width = math::max3(
        left_panel_->minimumSizeHint().width(),
        right_panel_->minimumSizeHint().width(),
        vert_splitter_->minimumSizeHint().width());
    hori_splitter_->setSizes(QList<int>(
        { hor_init_width, 2 * hor_init_width, hor_init_width }));
}

void Editor::set_display_image(const Image2D<Spectrum> &img)
{
    preview_window_->set_preview_image(img);
}

void Editor::launch_renderer(bool enable_preview)
{
    update_display_timer_->stop();

    if(preview_window_->is_in_realtime_mode())
    {
        renderer_.reset();
        return;
    }

    const int film_width  = preview_window_->width();
    const int film_height = preview_window_->height();

    if(film_width == 0 || film_height == 0)
    {
        renderer_.reset();
        return;
    }

    scene_->start_rendering();

    renderer_ = renderer_panel_->create_renderer(
        scene_, { film_width, film_height }, enable_preview);

    if(enable_preview)
        set_display_image(renderer_->start());
    else
        renderer_->start();

    update_display_timer_->start();
}

AGZ_EDITOR_END
