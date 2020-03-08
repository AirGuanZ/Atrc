#pragma once

#include <QCheckBox>
#include <QDialog>

#include <agz/editor/envir_light/envir_light.h>
#include <agz/editor/ui/global_setting_widget.h>

AGZ_EDITOR_BEGIN

class ObjectContext;
class PreviewWindow;
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
        PreviewWindow       *preview_window);

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
    QCheckBox *save_preview_window_  = nullptr;

    SceneManager        *scene_mgr_       = nullptr;
    ObjectContext       *obj_ctx_         = nullptr;
    EnvirLightSlot      *envir_light_     = nullptr;
    GlobalSettingWidget *global_settings_ = nullptr;
    PreviewWindow       *preview_window_  = nullptr;
};

AGZ_EDITOR_END
