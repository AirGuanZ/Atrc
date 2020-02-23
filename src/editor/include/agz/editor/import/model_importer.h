#pragma once

#include <QDialog>
#include <QPushButton>

#include <agz/editor/ui/utility/elided_label.h>

AGZ_EDITOR_BEGIN

class SceneManager;

class ModelImporter : public QDialog
{
public:

    ModelImporter(QWidget *parent, SceneManager *scene_mgr);

private:

    SceneManager *scene_mgr_;

    QPushButton *browse_filename_    = nullptr;
    ElidedLabel *filename_displayer_ = nullptr;

    QPushButton *ok_     = nullptr;
    QPushButton *cancel_ = nullptr;
};

AGZ_EDITOR_END
