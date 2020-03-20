#pragma once

#include <QCheckBox>
#include <QDialog>

#include <agz/editor/envir_light/envir_light.h>
#include <agz/editor/ui/global_setting_widget.h>

AGZ_EDITOR_BEGIN

class ObjectContext;
class PostProcessorSeq;
class PreviewWindow;
class RendererPanel;
class SceneManager;

class AssetSaveDialog : public QDialog
{
    Q_OBJECT

public:

    AssetSaveDialog(
        SceneManager        *scene_mgr,
        ObjectContext       *obj_ctx,
        EnvirLightSlot      *envir_light,
        GlobalSettingWidget *global_settings,
        PostProcessorSeq    *post_processors,
        PreviewWindow       *preview_window,
        RendererPanel       *renderer);

private:

    void init_ui();

    void ok();

    QCheckBox *save_all_pools_      = nullptr;
    QCheckBox *save_material_pool_  = nullptr;
    QCheckBox *save_medium_pool_    = nullptr;
    QCheckBox *save_geometry_pool_  = nullptr;
    QCheckBox *save_texture2d_pool_ = nullptr;
    QCheckBox *save_texture3d_pool_ = nullptr;

    QCheckBox *save_entities_        = nullptr;
    QCheckBox *save_envir_light_     = nullptr;
    QCheckBox *save_global_settings_ = nullptr;
    QCheckBox *save_post_processors_ = nullptr;
    QCheckBox *save_preview_window_  = nullptr;
    QCheckBox *save_renderer_panel_  = nullptr;

    SceneManager        *scene_mgr_       = nullptr;
    ObjectContext       *obj_ctx_         = nullptr;
    EnvirLightSlot      *envir_light_     = nullptr;
    GlobalSettingWidget *global_settings_ = nullptr;
    PostProcessorSeq    *post_processors_ = nullptr;
    PreviewWindow       *preview_window_  = nullptr;
    RendererPanel       *renderer_panel_  = nullptr;
};

AGZ_EDITOR_END
