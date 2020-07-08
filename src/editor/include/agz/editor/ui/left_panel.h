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
};

AGZ_EDITOR_END
