#pragma once

#include <QMainWindow>

#include <agz/editor/display_label.h>
#include <agz/editor/renderer/path_tracer.h>
#include <agz/utility/misc.h>

AGZ_EDITOR_BEGIN

class Editor : public QMainWindow, public misc::uncopyable_t
{
    Q_OBJECT

public:

    Editor();

    ~Editor();

private slots:

    void on_load_config();

    void on_update_time();

    void on_resize_display();

private:

    void load_config(const std::string &input_filename);

    void set_display_image(const Image2D<math::color3b> &img);

    std::unique_ptr<PathTracer> path_tracer_;

    std::shared_ptr<tracer::Scene>  scene_;
    std::shared_ptr<tracer::Camera> camera_;

    DisplayLabel *display_label_;
};

AGZ_EDITOR_END
