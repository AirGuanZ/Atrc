#pragma once

#include <QVBoxLayout>
#include <QWidget>

#include <agz/editor/common.h>

#include "ui_CameraPanel.h"

AGZ_EDITOR_BEGIN

class CameraPanel : public Ui::CameraPanel, public QWidget
{
public:
    
    explicit CameraPanel(QWidget *parent);
};

AGZ_EDITOR_END
