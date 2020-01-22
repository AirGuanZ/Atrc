#pragma once

#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>

#include <agz/gui/reporter.h>
#include <agz/tracer/factory/factory.h>
#include <agz/tracer/utility/render_session.h>

class GUI : public QMainWindow
{
    Q_OBJECT

public:

    GUI();

    ~GUI();

protected:

    void closeEvent(QCloseEvent *event) override;

private slots:

    void on_load_config();

    void on_stop_rendering();

    void on_update_preview();

    void on_update_pbar(double percent);

private:

    void start_rendering(const std::string &input_filename);

    void stop_rendering();

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

    QProgressBar *pbar_;
};
