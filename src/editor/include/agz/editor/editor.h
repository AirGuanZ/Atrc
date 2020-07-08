#pragma once

#include <QSplitter>

#include <agz/editor/displayer/preview_window.h>
#include <agz/editor/envir_light/envir_light.h>
#include <agz/editor/film_filter/film_filter.h>
#include <agz/editor/post_processor/post_processor_seq.h>
#include <agz/editor/renderer/renderer.h>
#include <agz/editor/renderer/renderer_widget.h>
#include <agz/editor/resource/resource.h>
#include <agz/editor/scene/scene_mgr.h>
#include <agz/editor/ui/down_panel.h>
#include <agz/editor/ui/left_panel.h>
#include <agz/editor/ui/right_panel.h>
#include <agz/gui_common/gui.h>
#include <agz/tracer/create/scene.h>
#include <agz/utility/misc.h>

AGZ_EDITOR_BEGIN

class GlobalSettingWidget;

class AssetLoadDialog;
class AssetSaveDialog;

class Editor : public QMainWindow, public misc::uncopyable_t
{
    Q_OBJECT

public:

    Editor();

    ~Editor();

    void on_change_camera();

    void on_change_aggregate();

    void add_to_resource_panel(QWidget *widget);

    void show_resource_panel(QWidget *widget, bool display_rsc_panel);

    void add_to_entity_panel(QWidget *widget);

    void show_entity_panel(QWidget *widget, bool display_entity_panel);

signals:

    void change_envir_light();

private:

    // slots

    void on_update_display();

    void on_change_renderer();

    void on_update_envir_light();

    // init

    void init_panels();

    void init_displayer();

    void init_obj_context();

    void init_scene_mgr();

    void init_renderer_panel();

    void init_global_setting_widget();

    void init_post_processor_widget();

    void init_film_filter();

    void init_save_asset_dialog();

    void init_render_menu();

    void redistribute_panels();

    // helper

    void set_display_image(const Image2D<Spectrum> &img);

    void launch_renderer(bool enable_preview);

    // renderer

    Box<Renderer> renderer_;

    RendererPanel *renderer_panel_ = nullptr;

    // scene data

    tracer::DefaultSceneParams scene_params_;
    RC<tracer::Scene> scene_;

    Box<SceneManager> scene_mgr_;

    // ui

    QFrame *up_panel_    = nullptr;
    
    DownPanel  *down_panel_  = nullptr;
    LeftPanel  *left_panel_  = nullptr;
    RightPanel *right_panel_ = nullptr;

    QPointer<QWidget> editing_entity_widget_;
    QPointer<QWidget> editing_rsc_widget_;

    QSplitter *hori_splitter_ = nullptr;
    QSplitter *vert_splitter_ = nullptr;

    // obj ctx

    Box<ObjectContext> obj_ctx_;

    // global setting

    GlobalSettingWidget *global_setting_ = nullptr;

    // post processors

    Box<PostProcessorSeq> pp_seq_;

    // film filter

    Box<FilmFilterPanel> film_filter_;

    // env light

    EnvirLightSlot *envir_light_slot_ = nullptr;

    // loader/saver

    Box<AssetLoadDialog> asset_load_dialog_;
    Box<AssetSaveDialog> asset_save_dialog_;

    // preview

    PreviewWindow *preview_window_ = nullptr;
    QTimer *update_display_timer_ = nullptr;

    Box<GUI> gui_render_window_;
};

AGZ_EDITOR_END
