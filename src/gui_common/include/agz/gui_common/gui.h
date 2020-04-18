#pragma once

#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>

#include <agz/gui_common/reporter.h>
#include <agz/factory/factory.h>
#include <agz/tracer/tracer.h>

class GUI : public QMainWindow
{
    Q_OBJECT

public:

    GUI();

    ~GUI();

    void load_config(const std::string &scene_filename);

protected:

    void closeEvent(QCloseEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

private slots:

    void on_load_config();

    void on_stop_rendering();

    void on_update_preview();

    void on_update_pbar(double percent);

    void on_exec_post_processor();

private:

    void start_rendering(const std::string &input_filename);

    void stop_rendering();

    void set_preview_img(const agz::tracer::Image2D<agz::tracer::Spectrum> &img);

    struct Context
    {
        agz::tracer::factory::CreatingContext context;
        std::unique_ptr<agz::tracer::factory::PathMapper> path_mapper;
        agz::tracer::ConfigGroup root_params;
    };

    std::unique_ptr<Context> render_context_;
    agz::tracer::RenderSession render_session_;
    std::shared_ptr<GUIProgressReporter> reporter_;

    QLabel *preview_label_;
    QPixmap pixmap_;

    QProgressBar *pbar_;
};
