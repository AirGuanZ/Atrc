#include <QFileDialog>
#include <QMenuBar>
#include <QWidgetAction>

#include <agz/editor/editor.h>
#include <agz/editor/imexport/asset_load_dialog.h>
#include <agz/editor/imexport/asset_save_dialog.h>
#include <agz/editor/renderer/renderer_widget.h>
#include <agz/editor/ui/global_setting_widget.h>
#include <agz/editor/ui/log_widget.h>

#include <agz/utility/file.h>

#define WORKING_DIR_PATH_NAME "${working-directory}"
#define SCENE_DESC_PATH_NAME  "${scene-directory}"

AGZ_EDITOR_BEGIN

Editor::Editor()
{
    init_panels();

    init_log_widget();

    AGZ_INFO(">>> Atrc Scene Editor by AirGuanZ <<<");
    
#ifdef USE_EMBREE
    AGZ_INFO("Embree is enabled");
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

    AGZ_INFO("initialize global settings");

    init_global_setting_widget();

    redistribute_panels();

    AGZ_INFO("initialize initial scene");

    scene_params_.envir_light = envir_light_slot_->get_tracer_object();
    scene_params_.aggregate = scene_mgr_->update_tracer_aggregate(scene_params_.entities);
    scene_ = create_default_scene(scene_params_);
    scene_->set_camera(displayer_->create_camera());

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
}

void Editor::on_change_camera()
{
    if(!scene_)
        return;

    std::thread destroy_renderer_thread([renderer = std::move(renderer_)] { });
    destroy_renderer_thread.detach();
    
    scene_ = create_default_scene(scene_params_);
    auto camera = displayer_->create_camera();
    scene_->set_camera(camera);

    launch_renderer(true);
}

void Editor::on_change_aggregate()
{
    auto old_camera = scene_->get_shared_camera();

    renderer_.reset();
    scene_params_.entities.clear();
    scene_params_.aggregate = scene_mgr_->update_tracer_aggregate(scene_params_.entities);
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
        right_panel_->right_tab_widget->setCurrentWidget(right_panel_->resource_area);
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
        right_panel_->right_tab_widget->setCurrentWidget(right_panel_->entity_area);
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
    down_panel_ = new QFrame(vert_splitter_);

    up_panel_->setFrameShape(QFrame::Box);

    vert_splitter_->setOrientation(Qt::Vertical);
    vert_splitter_->addWidget(up_panel_);
    vert_splitter_->addWidget(down_panel_);
}

void Editor::init_log_widget()
{
    LogWidget *log_widget          = new LogWidget;
    QVBoxLayout *down_panel_layout = new QVBoxLayout(down_panel_);
    down_panel_layout->addWidget(log_widget);

    auto sink = std::make_shared<LogWidgetSink>();
    sink->log_widget = log_widget;
    spdlog::default_logger()->sinks().push_back(std::move(sink));
}

void Editor::init_displayer()
{
    displayer_ = new Displayer(up_panel_, this);
    connect(displayer_, &Displayer::need_to_recreate_camera, [=] { on_change_camera(); });

    QVBoxLayout *up_panel_layout = new QVBoxLayout(up_panel_);
    up_panel_layout->addWidget(displayer_);

    connect(displayer_->get_gl_widget(), &DisplayerGLWidget::switch_render_mode, [=]
    {
        renderer_.reset();
        launch_renderer(true);
    });

    update_display_timer_ = new QTimer(this);
    update_display_timer_->setInterval(200);
    connect(update_display_timer_, &QTimer::timeout, [=] { on_update_display(); });
    update_display_timer_->start();
}

void Editor::init_obj_context()
{
    obj_ctx_ = std::make_unique<ObjectContext>(this);

    left_panel_->material_tab_layout->addWidget(
        obj_ctx_->pool<tracer::Material>()->get_widget());

    left_panel_->medium_tab_layout->addWidget(
        obj_ctx_->pool<tracer::Medium>()->get_widget());

    left_panel_->texture2d_tab_layout->addWidget(
        obj_ctx_->pool<tracer::Texture2D>()->get_widget());

    left_panel_->texture3d_tab_layout->addWidget(
        obj_ctx_->pool<tracer::Texture3D>()->get_widget());

    left_panel_->geometry_tab_layout->addWidget(
        obj_ctx_->pool<tracer::Geometry>()->get_widget());
}

void Editor::init_scene_mgr()
{
    scene_mgr_ = std::make_unique<SceneManager>(*obj_ctx_, this, displayer_);
    left_panel_->up_panel_layout->addWidget(scene_mgr_->get_widget());

    connect(scene_mgr_.get(), &SceneManager::change_scene, [=]
    {
        on_change_aggregate();
    });
}

void Editor::init_renderer_panel()
{
    // renderer

    renderer_panel_ = new RendererPanel(right_panel_->renderer_tab, "Path Tracer");
    right_panel_->renderer_tab_layout->addWidget(renderer_panel_);

    connect(renderer_panel_, &RendererPanel::change_renderer_params, [=] { on_change_renderer(); });
    connect(renderer_panel_, &RendererPanel::change_renderer_type, [=] { on_change_renderer(); });

    // camera

    right_panel_->camera_tab_layout->addWidget(displayer_->get_camera_panel());

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

    connect(global_setting_->camera_rotate_speed, &QSlider::valueChanged,
        [=](int v)
    {
        const real speed = real(0.001) * v / 10;
        global_setting_->display_camera_rotate_speed->setText(QString::number(100 * speed, 'g', 2));
        displayer_->set_camera_rotate_speed(speed);
    });

    global_setting_->camera_rotate_speed->setRange(1, 100);
    global_setting_->camera_rotate_speed->setValue(40);
}

void Editor::init_save_asset_dialog()
{
    AssetLoadDialog *load_dialog = new AssetLoadDialog(
        scene_mgr_.get(), obj_ctx_.get(), envir_light_slot_);
    QAction *load_action = new QAction("Load", this);
    menuBar()->addAction(load_action);
    connect(load_action, &QAction::triggered, [=]
    {
        load_dialog->exec();
        if(!load_dialog->is_ok_clicked())
            return;

        renderer_.reset();

        scene_params_.envir_light = envir_light_slot_->get_tracer_object();
        scene_params_.aggregate = scene_mgr_->update_tracer_aggregate(scene_params_.entities);

        scene_ = create_default_scene(scene_params_);
        scene_->set_camera(displayer_->create_camera());

        launch_renderer(true);
    });

    AssetSaveDialog *save_dialog = new AssetSaveDialog(
        scene_mgr_.get(), obj_ctx_.get(), envir_light_slot_);
    QAction *save_action = new QAction("Save", this);
    menuBar()->addAction(save_action);
    connect(save_action, &QAction::triggered, save_dialog, &AssetSaveDialog::exec);
}

void Editor::redistribute_panels()
{
    const int ver_init_height = (std::max)(displayer_->minimumSizeHint().height(),
                                     down_panel_->minimumSizeHint().height());
    vert_splitter_->setSizes(QList<int>({ 2 * ver_init_height, ver_init_height }));

    const int hor_init_width = math::max3(left_panel_->minimumSizeHint().width(),
                                          right_panel_->minimumSizeHint().width(),
                                          vert_splitter_->minimumSizeHint().width());
    hori_splitter_->setSizes(QList<int>({ hor_init_width, 2 * hor_init_width, hor_init_width }));
}

void Editor::load_config(const std::string &input_filename)
{
    using namespace tracer;

    renderer_.reset();

    AGZ_INFO("load JSON config from {}", input_filename);

    factory::BasicPathMapper path_mapper;
    {
        const auto working_dir = absolute(
            std::filesystem::current_path()).lexically_normal().string();
        path_mapper.add_replacer(WORKING_DIR_PATH_NAME, working_dir);

        const auto scene_dir = absolute(
            std::filesystem::path(input_filename)).parent_path().lexically_normal().string();
        path_mapper.add_replacer(SCENE_DESC_PATH_NAME, scene_dir);
    }

    const std::string input_content = file::read_txt_file(input_filename);
    const ConfigGroup root_params = json_to_config(string_to_json(input_content));

    const ConfigGroup &scene_config = root_params.child_group("scene");

    factory::CreatingContext context;
    context.path_mapper = &path_mapper;
    context.reference_root = &scene_config;
    
    displayer_->load_camera_from_config(root_params.child_group("rendering").child_group("camera"));
    auto camera = displayer_->create_camera();

    {
        scene_params_ = DefaultSceneParams();

        if(auto ent_arr = scene_config.find_child_array("entities"))
        {
            if(ent_arr->size() == 1)
                AGZ_INFO("creating 1 entity");
            else
                AGZ_INFO("creating {} entities", ent_arr->size());

            for(size_t i = 0; i < ent_arr->size(); ++i)
            {
                auto &group = ent_arr->at_group(i);
                if(stdstr::ends_with(group.child_str("type"), "//"))
                {
                    AGZ_INFO("skip entity with type ending with //");
                    continue;
                }

                auto ent = context.create<Entity>(group);
                scene_params_.entities.push_back(ent);
            }
        }

        if(auto group = scene_config.find_child_group("env"))
            scene_params_.envir_light = context.create<EnvirLight>(*group);

        if(auto group = scene_config.find_child_group("aggregate"))
            scene_params_.aggregate = context.create<Aggregate>(*group);
        else
            scene_params_.aggregate = create_native_aggregate();

        std::vector<std::shared_ptr<const Entity>> const_entities;
        const_entities.reserve(scene_params_.entities.size());
        for(auto ent : scene_params_.entities)
            const_entities.push_back(ent);
        scene_params_.aggregate->build(const_entities);
    }

    scene_ = create_default_scene(scene_params_);
    scene_->set_camera(camera);

    launch_renderer(true);
}

void Editor::set_display_image(const Image2D<Spectrum> &img)
{
    displayer_->set_display_image(img);
}

void Editor::launch_renderer(bool enable_preview)
{
    update_display_timer_->stop();

    if(displayer_->is_in_realtime_mode())
    {
        renderer_.reset();
        return;
    }

    const int film_width  = displayer_->size().width();
    const int film_height = displayer_->size().height();

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
