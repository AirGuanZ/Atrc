#pragma once

#include <QScrollArea>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class RightPanel : public QWidget
{
public:

    explicit RightPanel(QWidget *parent);

    QTabWidget *right_tab_widget = nullptr;

    QScrollArea *entity_area      = nullptr;
    QScrollArea *resource_area    = nullptr;
    QScrollArea *renderer_area    = nullptr;
    QScrollArea *envir_light_area = nullptr;
    QScrollArea *camera_area      = nullptr;

    QWidget *entity_tab   = nullptr;
    QWidget *resource_tab = nullptr;
    
    QWidget *renderer_tab    = nullptr;
    QWidget *envir_light_tab = nullptr;
    QWidget *camera_tab      = nullptr;

    QVBoxLayout *entity_tab_layout      = nullptr;
    QVBoxLayout *resource_tab_layout    = nullptr;
    QVBoxLayout *renderer_tab_layout    = nullptr;
    QVBoxLayout *envir_light_tab_layout = nullptr;
    QVBoxLayout *camera_tab_layout      = nullptr;
};

AGZ_EDITOR_END
