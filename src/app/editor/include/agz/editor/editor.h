#pragma once

#include <QMainWindow>
#include <QSplitter>

#include <agz/editor/displayer/camera_panel.h>
#include <agz/editor/displayer/displayer.h>
#include <agz/editor/renderer/renderer.h>
#include <agz/editor/renderer/renderer_widget.h>
#include <agz/utility/misc.h>

AGZ_EDITOR_BEGIN

class Editor : public QMainWindow, public misc::uncopyable_t
{
    Q_OBJECT

public:

    Editor();

    ~Editor();

private:

    // slots

    void on_load_config();

    void on_update_display();

    void on_change_camera();

    void on_change_renderer();

    // init

    void init_menu_bar();

    void init_panels();

    void init_displayer();

    void init_renderer_panel();

    void init_camera_panel();

    void redistribute_panels();

    // helper

    void load_config(const std::string &input_filename);

    void set_display_image(const Image2D<math::color3b> &img);

    void launch_renderer();

    std::unique_ptr<Renderer> renderer_;

    std::shared_ptr<tracer::Scene> scene_;

    QFrame *left_panel_  = nullptr;
    QFrame *up_panel_    = nullptr;
    QFrame *down_panel_  = nullptr;
    QFrame *right_panel_ = nullptr;

    QVBoxLayout *right_panel_layout_ = nullptr;

    QSplitter *hori_splitter_ = nullptr;
    QSplitter *vert_splitter_ = nullptr;

    RendererPanel *renderer_panel_ = nullptr;
    QWidget       *camera_panel_ = nullptr;

    Displayer *displayer_ = nullptr;
    QTimer *update_display_timer_ = nullptr;
};

AGZ_EDITOR_END
