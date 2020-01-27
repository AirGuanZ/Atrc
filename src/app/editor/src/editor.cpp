#include <QFileDialog>
#include <QGridLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QTimer>

#include <agz/editor/editor.h>
#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/factory/factory.h>
#include <agz/tracer/utility/config_cvt.h>
#include <agz/tracer/utility/render_session.h>
#include <agz/utility/file.h>

#ifdef USE_EMBREE
#   include <agz/tracer/utility/embree.h>
#endif

#define WORKING_DIR_PATH_NAME "${working-directory}"
#define SCENE_DESC_PATH_NAME  "${scene-directory}"

AGZ_EDITOR_BEGIN

Editor::Editor()
{
    QAction *load_config = new QAction("Load JSON Config", this);
    menuBar()->addAction(load_config);
    connect(load_config, &QAction::triggered, this, &Editor::on_load_config);

    QWidget *central_widget = new QWidget(this);
    setCentralWidget(central_widget);

    QGridLayout *layout = new QGridLayout(central_widget);
    central_widget->setLayout(layout);

    display_label_ = new DisplayLabel(this);
    connect(display_label_, &DisplayLabel::resize, this, &Editor::on_resize_display);
    layout->addWidget(display_label_, 0, 0);

#ifdef USE_EMBREE
    tracer::init_embree_device();
#endif

    QTimer *update_timer = new QTimer(this);
    update_timer->setInterval(100);
    connect(update_timer, &QTimer::timeout, this, &Editor::on_update_time);
    update_timer->start();
}

Editor::~Editor()
{
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
        path_tracer_.reset();

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
        path_tracer_.reset();

        QMessageBox mbox;
        mbox.setWindowTitle("Error");
        mbox.setText("An unknown error occurred");
        mbox.exec();
    }
}

void Editor::on_update_time()
{
    if(path_tracer_)
    {
        const auto img = path_tracer_->get_image();
        if(img.is_available())
            set_display_image(img);
    }
}

void Editor::on_resize_display()
{
    if(!scene_)
        return;

    path_tracer_.reset();

    const int film_width = display_label_->size().width();
    const int film_height = display_label_->size().height();

    camera_->update_param("film_width", film_width);
    camera_->update_param("film_height", film_height);

    scene_->start_rendering();

    path_tracer_ = std::make_unique<PathTracer>(PathTracingParams{}, film_width, film_height, scene_);
}

void Editor::load_config(const std::string &input_filename)
{
    using namespace tracer;

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

    scene_ = context.create<Scene>(scene_config);

    const int film_width = display_label_->size().width();
    const int film_height = display_label_->size().height();
    
    camera_ = context.create<Camera>(root_params.child_group("rendering").child_group("camera"), film_width, film_height);
    
    scene_->set_camera(camera_);
    scene_->start_rendering();

    path_tracer_ = std::make_unique<PathTracer>(PathTracingParams{}, film_width, film_height, scene_);
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

    display_label_->setPixmap(qpixmap);
}

AGZ_EDITOR_END
