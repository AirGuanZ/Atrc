#pragma once

#include <QMainWindow>
#include <QSplitter>

#include <agz/editor/displayer/displayer.h>
#include <agz/editor/envir_light/envir_light.h>
#include <agz/editor/renderer/renderer.h>
#include <agz/editor/renderer/renderer_widget.h>
#include <agz/editor/resource/resource.h>
#include <agz/editor/scene/scene_mgr.h>
#include <agz/editor/ui/left_panel.h>
#include <agz/editor/ui/right_panel.h>
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

    void init_log_widget();

    void init_displayer();

    void init_obj_context();

    void init_scene_mgr();

    void init_renderer_panel();

    void init_global_setting_widget();

    void init_save_asset_dialog();

    void redistribute_panels();

    // helper

    void load_config(const std::string &input_filename);

    void set_display_image(const Image2D<Spectrum> &img);

    void launch_renderer(bool enable_preview);

    std::unique_ptr<Renderer> renderer_;

    tracer::DefaultSceneParams scene_params_;
    std::shared_ptr<tracer::Scene> scene_;

    QFrame *up_panel_    = nullptr;
    QFrame *down_panel_  = nullptr;

    LeftPanel  *left_panel_  = nullptr;
    RightPanel *right_panel_ = nullptr;

    QPointer<QWidget> editing_entity_widget_;
    QPointer<QWidget> editing_rsc_widget_;

    std::unique_ptr<ObjectContext> obj_ctx_;

    GlobalSettingWidget *global_setting_ = nullptr;

    EnvirLightSlot *envir_light_slot_ = nullptr;

    std::unique_ptr<AssetLoadDialog> asset_load_dialog_;
    std::unique_ptr<AssetSaveDialog> asset_save_dialog_;

    std::unique_ptr<SceneManager> scene_mgr_;
    
    QSplitter *hori_splitter_ = nullptr;
    QSplitter *vert_splitter_ = nullptr;

    RendererPanel *renderer_panel_ = nullptr;

    Displayer *displayer_ = nullptr;
    QTimer *update_display_timer_ = nullptr;
};

AGZ_EDITOR_END
