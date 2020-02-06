#pragma once

#include <QMainWindow>
#include <QSplitter>

#include <agz/editor/displayer/displayer.h>
#include <agz/editor/renderer/renderer.h>
#include <agz/editor/renderer/renderer_widget.h>
#include <agz/editor/texture2d/texture2d.h>
#include <agz/editor/ui/right_panel.h>
#include <agz/utility/misc.h>

AGZ_EDITOR_BEGIN

class Editor : public QMainWindow, public misc::uncopyable_t
{
    Q_OBJECT

public:

    Editor();

    ~Editor();

    void on_change_camera();

    void add_to_resource_panel(QWidget *widget);

    void show_resource_panel(QWidget *widget);

private:

    // slots

    void on_load_config();

    void on_update_display();

    void on_change_renderer();

    // init

    void init_menu_bar();

    void init_panels();

    void init_displayer();

    void init_resource_panel();

    void init_texture2d_pool();

    void init_renderer_panel();

    void init_camera_panel();

    void init_global_setting_widget();

    void redistribute_panels();

    // helper

    void load_config(const std::string &input_filename);

    void set_display_image(const Image2D<math::color3b> &img);

    void launch_renderer();

    std::unique_ptr<Renderer> renderer_;

    tracer::DefaultSceneParams scene_params_;
    std::shared_ptr<tracer::Scene> scene_;

    QFrame *left_panel_  = nullptr;
    QFrame *up_panel_    = nullptr;
    QFrame *down_panel_  = nullptr;

    RightPanel *right_panel_ = nullptr;

    QPointer<QWidget> editing_rsc_widget_;

    Texture2DWidgetFactory texture2d_factory_;
    Texture2DPool *texture2d_pool_ = nullptr;
    
    QSplitter *hori_splitter_ = nullptr;
    QSplitter *vert_splitter_ = nullptr;

    RendererPanel *renderer_panel_ = nullptr;

    Displayer *displayer_ = nullptr;
    QTimer *update_display_timer_ = nullptr;
};

AGZ_EDITOR_END
