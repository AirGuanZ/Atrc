#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QTimer>

#include <agz/editor/editor.h>
#include <agz/editor/renderer/renderer_widget.h>

#include <agz/utility/file.h>

#define WORKING_DIR_PATH_NAME "${working-directory}"
#define SCENE_DESC_PATH_NAME  "${scene-directory}"

AGZ_EDITOR_BEGIN

Editor::Editor()
{
#ifdef USE_EMBREE
    tracer::init_embree_device();
#endif

    init_menu_bar();

    init_panels();

    init_displayer();

    init_renderer_panel();

    init_camera_panel();

    redistribute_panels();
}

Editor::~Editor()
{
    renderer_.reset();
    scene_.reset();

#ifdef USE_EMBREE
    tracer::destroy_embree_device();
#endif
}

void Editor::on_load_config()
{
    try
    {
        const std::string input_filename = QFileDialog::getOpenFileName(
            this, "Configuration File").toStdString();
        if(!input_filename.empty())
            load_config(input_filename);
    }
    catch(const std::exception & err)
    {
        renderer_.reset();

        QMessageBox mbox;
        mbox.setWindowTitle("Error");

        std::vector<std::string> err_strs;
        misc::extract_hierarchy_exceptions(err, std::back_inserter(err_strs));

        QString err_msg;
        for(auto &s : err_strs)
            err_msg.append(QString::fromStdString(s + "\n"));

        mbox.setText(err_msg);
        mbox.exec();
    }
    catch(...)
    {
        renderer_.reset();

        QMessageBox mbox;
        mbox.setWindowTitle("Error");
        mbox.setText("An unknown error occurred");
        mbox.exec();
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

void Editor::on_change_camera()
{
    if(!scene_)
        return;

    renderer_.reset();
    
    auto camera = displayer_->create_camera();
    scene_->set_camera(camera);

    launch_renderer();
}

void Editor::on_change_renderer()
{
    if(!scene_)
        return;

    renderer_.reset();

    launch_renderer();
}

void Editor::init_menu_bar()
{
    QAction *load_config = new QAction("Load JSON Config", this);
    menuBar()->addAction(load_config);
    connect(load_config, &QAction::triggered, [=] { on_load_config(); });
}

void Editor::init_panels()
{
    hori_splitter_ = new QSplitter(this);
    setCentralWidget(hori_splitter_);

    left_panel_    = new QFrame(hori_splitter_);
    vert_splitter_ = new QSplitter(hori_splitter_);
    right_panel_   = new QFrame(hori_splitter_);

    left_panel_->setFrameShape(QFrame::Box);
    left_panel_->setFrameShadow(QFrame::Shadow::Sunken);

    right_panel_->setFrameShape(QFrame::Box);
    right_panel_->setFrameShadow(QFrame::Shadow::Sunken);

    right_panel_layout_ = new QVBoxLayout(right_panel_);
    right_panel_layout_->setAlignment(Qt::AlignTop);

    hori_splitter_->setOrientation(Qt::Horizontal);
    hori_splitter_->addWidget(left_panel_);
    hori_splitter_->addWidget(vert_splitter_);
    hori_splitter_->addWidget(right_panel_);

    up_panel_   = new QFrame(vert_splitter_);
    down_panel_ = new QFrame(vert_splitter_);

    up_panel_->setFrameShape(QFrame::Box);
    up_panel_->setFrameShadow(QFrame::Shadow::Sunken);
    
    down_panel_->setFrameShape(QFrame::Box);
    down_panel_->setFrameShadow(QFrame::Shadow::Sunken);

    vert_splitter_->setOrientation(Qt::Vertical);
    vert_splitter_->addWidget(up_panel_);
    vert_splitter_->addWidget(down_panel_);
}

void Editor::init_displayer()
{
    displayer_ = new Displayer(up_panel_);
    connect(displayer_, &Displayer::need_to_recreate_camera, [=] { on_change_camera(); });

    QVBoxLayout *up_panel_layout = new QVBoxLayout(up_panel_);
    up_panel_layout->addWidget(displayer_);

    update_display_timer_ = new QTimer(this);
    update_display_timer_->setInterval(200);
    connect(update_display_timer_, &QTimer::timeout, [=] { on_update_display(); });
    update_display_timer_->start();
}

void Editor::init_renderer_panel()
{
    renderer_panel_ = new RendererPanel(right_panel_, "Path Tracer");
    right_panel_layout_->addWidget(renderer_panel_);

    connect(renderer_panel_, &RendererPanel::change_renderer_params, [=] { on_change_renderer(); });
    connect(renderer_panel_, &RendererPanel::change_renderer_type, [=] { on_change_renderer(); });
}

void Editor::init_camera_panel()
{
    camera_panel_ = displayer_->get_camera_panel();
    right_panel_layout_->addWidget(camera_panel_);
}

void Editor::redistribute_panels()
{
    auto ver_init_height = (std::max)(displayer_->minimumSizeHint().height(),
                                   down_panel_->minimumSizeHint().height());
    vert_splitter_->setSizes(QList<int>({ 2 * ver_init_height, ver_init_height }));

    auto hor_init_width = (std::max)((std::max)(left_panel_->minimumSizeHint().width(),
        right_panel_->minimumSizeHint().width()),
        vert_splitter_->minimumSizeHint().width());
    hori_splitter_->setSizes(QList<int>({ hor_init_width, 2 * hor_init_width, hor_init_width }));
}

void Editor::load_config(const std::string &input_filename)
{
    using namespace tracer;

    renderer_.reset();
    scene_.reset();

    factory::BasicPathMapper path_mapper;
    {
        const auto working_dir = absolute(std::filesystem::current_path()).lexically_normal().string();
        path_mapper.add_replacer(WORKING_DIR_PATH_NAME, working_dir);

        const auto scene_dir = absolute(std::filesystem::path(input_filename)).parent_path().lexically_normal().string();
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

    scene_ = context.create<Scene>(scene_config);
    scene_->set_camera(camera);

    launch_renderer();
}

void Editor::set_display_image(const Image2D<math::color3b> &img)
{
    const QImage qimg(
        reinterpret_cast<const unsigned char *>(img.raw_data()),
        img.width(), img.height(),
        sizeof(math::color3b) * img.width(),
        QImage::Format::Format_RGB888);

    QPixmap qpixmap;
    qpixmap.convertFromImage(qimg);

    displayer_->setPixmap(qpixmap);
}

void Editor::launch_renderer()
{
    update_display_timer_->stop();

    const int film_width = displayer_->size().width();
    const int film_height = displayer_->size().height();

    scene_->start_rendering();
    renderer_ = renderer_panel_->create_renderer(scene_, { film_width, film_height });

    connect(renderer_.get(), &Renderer::can_get_img, this, &Editor::on_update_display);

    renderer_->start();

    update_display_timer_->start();
}

AGZ_EDITOR_END
