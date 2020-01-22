#include <filesystem>

#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QVBoxLayout>
#include <QWidget>

#include <agz/gui/gui.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/utility/config_cvt.h>
#include <agz/utility/file.h>
#include <agz/utility/misc.h>
#include <agz/utility/string.h>

#ifdef USE_EMBREE
#   include <agz/tracer/utility/embree.h>
#endif

#define WORKING_DIR_PATH_NAME "${working-directory}"
#define SCENE_DESC_PATH_NAME  "${scene-directory}"

class PathMapper : public agz::tracer::factory::PathMapper
{
    std::map<std::string, std::string> replacers_;

public:

    void add_replacer(const std::string &key, const std::string &value)
    {
        replacers_[key] = value;
    }

    std::string map(const std::string &s) const override
    {
        std::string ret(s);
        for(auto &p : replacers_)
            agz::stdstr::replace_(ret, p.first, p.second);
        return absolute(std::filesystem::path(ret)).lexically_normal().string();
    }
};

GUI::GUI()
{
    QMenuBar *menu_bar = menuBar();

    QAction *load = new QAction("Open", this);
    menu_bar->addAction(load);
    connect(load, &QAction::triggered, this, &GUI::on_load_config);

    QAction *stop = new QAction("Stop", this);
    menu_bar->addAction(stop);
    connect(stop, &QAction::triggered, this, &GUI::on_stop_rendering);

    QWidget *central_widget = new QWidget(this);
    setCentralWidget(central_widget);

    QVBoxLayout *layout = new QVBoxLayout(central_widget);
    central_widget->setLayout(layout);

    preview_label_ = new QLabel("Preview", central_widget);
    preview_label_->setAlignment(Qt::AlignCenter);
    layout->addWidget(preview_label_);

    pbar_ = new QProgressBar(central_widget);
    pbar_->setRange(0, 100);
    pbar_->setValue(0);
    layout->addWidget(pbar_);

#ifdef USE_EMBREE
    agz::tracer::init_embree_device();
#endif
}

GUI::~GUI()
{
#ifdef USE_EMBREE
    agz::tracer::destroy_embree_device();
#endif
}

void GUI::closeEvent(QCloseEvent *event)
{
    stop_rendering();
}

void GUI::on_load_config()
{
    stop_rendering();

    try
    {
        std::string input_filename = QFileDialog::getOpenFileName(
            this, "Configuration File").toStdString();

        start_rendering(input_filename);
    }
    catch(const std::exception &err)
    {
        stop_rendering();

        QMessageBox mbox;
        mbox.setWindowTitle("Error");

        std::vector<std::string> err_strs;
        agz::misc::extract_hierarchy_exceptions(err, std::back_inserter(err_strs));

        QString err_msg;
        for(auto &s : err_strs)
            err_msg.append(QString::fromStdString(s + "\n"));

        mbox.setText(err_msg);
        mbox.exec();
    }
    catch(...)
    {
        stop_rendering();

        QMessageBox mbox;
        mbox.setWindowTitle("Error");
        mbox.setText("An unknown error occurred");
        mbox.exec();
    }
}

void GUI::on_stop_rendering()
{
    stop_rendering();
}

void GUI::on_update_preview()
{
    if(reporter_)
    {
        if(auto img = reporter_->get_preview_image(); img.is_available())
        {
            agz::texture::texture2d_t<agz::math::color3b> imgu8(img.height(), img.width());
            for(int y = 0; y < imgu8.height(); ++y)
            {
                const int ry = img.height() - 1 - y;
                for(int x = 0; x < imgu8.width(); ++x)
                {
                    imgu8(y, x) = to_color3b(img(ry, x).map([](agz::tracer::real r)
                    {
                        return std::pow(r, agz::tracer::real(1 / 2.2));
                    }));
                }
            }

            QImage qimg(
                reinterpret_cast<unsigned char *>(imgu8.raw_data()),
                imgu8.width(), imgu8.height(),
                sizeof(agz::math::color3b) * imgu8.width(), QImage::Format::Format_RGB888);

            QPixmap qpixmap;
            qpixmap.convertFromImage(qimg);
            preview_label_->setPixmap(qpixmap);
        }
    }
}

void GUI::on_update_pbar(double percent)
{
    pbar_->setValue(agz::math::clamp<int>(static_cast<int>(percent), 0, 100));
}

void GUI::start_rendering(const std::string &input_filename)
{
    render_context_ = std::make_unique<Context>();

    std::unique_ptr<PathMapper> path_mapper = std::make_unique<PathMapper>();
    {
        const auto working_dir = absolute(std::filesystem::current_path()).lexically_normal().string();
        path_mapper->add_replacer(WORKING_DIR_PATH_NAME, working_dir);

        const auto scene_dir = absolute(std::filesystem::path(input_filename)).parent_path().lexically_normal().string();
        path_mapper->add_replacer(SCENE_DESC_PATH_NAME, scene_dir);
    }
    render_context_->path_mapper = std::move(path_mapper);
    render_context_->context.path_mapper = render_context_->path_mapper.get();

    std::string input_content = agz::file::read_txt_file(input_filename);
    render_context_->root_params = agz::tracer::json_to_config(agz::tracer::string_to_json(input_content));

    const auto &scene_config = render_context_->root_params.child_group("scene");
    render_context_->context.reference_root = &scene_config;

    auto scene = render_context_->context.create<agz::tracer::Scene>(scene_config);

    const auto rendering_config = render_context_->root_params.child_group("rendering");
    if(!rendering_config.is_group())
        throw std::runtime_error("rendering setting array is not supported by Atrc GUI");

    render_session_ = create_render_session(scene, rendering_config, render_context_->context);

    reporter_ = std::make_shared<GUIProgressReporter>(
        std::chrono::duration_cast<GUIProgressReporter::Clock::duration>(std::chrono::milliseconds(250)));
    render_session_.render_settings->reporter = reporter_;

    agz::tracer::FilmFilterApplier film_filter_applier(
        render_session_.render_settings->width,
        render_session_.render_settings->height,
        render_session_.render_settings->film_filter);

    scene->set_camera(render_session_.render_settings->camera);
    scene->start_rendering();

    connect(reporter_.get(), &GUIProgressReporter::update_preview, this, &GUI::on_update_preview);
    connect(reporter_.get(), &GUIProgressReporter::update_pbar, this, &GUI::on_update_pbar);

    render_session_.render_settings->renderer->render_async(
        film_filter_applier,
        *render_session_.scene,
        *render_session_.render_settings->reporter);
}

void GUI::stop_rendering()
{
    if(render_session_.render_settings)
        render_session_.render_settings->renderer->stop_async();

    render_context_.reset();
    render_session_ = agz::tracer::RenderSession();
    reporter_.reset();
}
