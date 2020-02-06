#pragma once

#include <QVBoxLayout>
#include <QWidget>

#include <agz/editor/common.h>

#include "ui_RightPanel.h"

AGZ_EDITOR_BEGIN

class RightPanel : public QWidget, public Ui::RightPanel
{
public:

    explicit RightPanel(QWidget *parent);

    QVBoxLayout *resource_tab_layout = nullptr;
    QVBoxLayout *renderer_tab_layout = nullptr;
    QVBoxLayout *camera_tab_layout   = nullptr;
};

AGZ_EDITOR_END
