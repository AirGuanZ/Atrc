#pragma once

#include <QDialog>

#include <agz/editor/envir_light/envir_light.h>
#include <agz/editor/imexport/json_export_context.h>

AGZ_EDITOR_BEGIN

class GlobalSettingWidget;
class ObjectContext;
class PreviewWindow;
class RendererPanel;
class SceneManager;

/*class JSONExportDialog : public QDialog
{
public:

    JSONExportDialog(
        SceneManager        *scene_mgr,
        ObjectContext       *obj_ctx,
        EnvirLightSlot      *envir_light,
        PreviewWindow       *preview_window,
        GlobalSettingWidget *global_settings,
        RendererPanel       *renderer_panel);

private:
    
    SceneManager        *scene_mgr_       = nullptr;
    ObjectContext       *obj_ctx_         = nullptr;
    EnvirLightSlot      *envir_light_     = nullptr;
    PreviewWindow       *preview_window_  = nullptr;
    GlobalSettingWidget *global_settings_ = nullptr;
    RendererPanel       *renderer_panel_  = nullptr;
};*/

void export_json(
    SceneManager        *scene_mgr,
    ObjectContext       *obj_ctx,
    EnvirLightSlot      *envir_light,
    PreviewWindow       *preview_window,
    GlobalSettingWidget *global_settings,
    RendererPanel       *renderer_panel);

AGZ_EDITOR_END
