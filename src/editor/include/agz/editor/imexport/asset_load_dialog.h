#pragma once

#include <QDialog>

#include <agz/editor/envir_light/envir_light.h>
#include <agz/editor/ui/global_setting_widget.h>

AGZ_EDITOR_BEGIN

class ObjectContext;
class SceneManager;

class AssetLoadDialog : public QDialog
{
    Q_OBJECT

public:

    AssetLoadDialog(
        SceneManager *scene_mgr,
        ObjectContext *obj_ctx,
        EnvirLightSlot *envir_light);

    bool is_ok_clicked() const noexcept;

private:

    void init_ui();

    void ok();

    bool is_ok_clicked_ = false;

    QCheckBox *load_envir_light_ = nullptr;

    SceneManager   *scene_mgr_   = nullptr;
    ObjectContext  *obj_ctx_     = nullptr;
    EnvirLightSlot *envir_light_ = nullptr;
};

AGZ_EDITOR_END
