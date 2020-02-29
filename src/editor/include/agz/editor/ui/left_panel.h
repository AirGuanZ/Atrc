#pragma once

#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class LeftPanel : public QWidget
{
public:

    explicit LeftPanel(QWidget *parent);

    QWidget     *up_panel        = nullptr;
    QVBoxLayout *up_panel_layout = nullptr;

    QTabWidget *down_tab   = nullptr;
    QWidget *material_tab  = nullptr;
    QWidget *medium_tab    = nullptr;
    QWidget *geometry_tab  = nullptr;
    QWidget *texture2d_tab = nullptr;
    QWidget *texture3d_tab = nullptr;

    QVBoxLayout *material_tab_layout  = nullptr;
    QVBoxLayout *medium_tab_layout    = nullptr;
    QVBoxLayout *geometry_tab_layout  = nullptr;
    QVBoxLayout *texture2d_tab_layout = nullptr;
    QVBoxLayout *texture3d_tab_layout = nullptr;
};

AGZ_EDITOR_END
