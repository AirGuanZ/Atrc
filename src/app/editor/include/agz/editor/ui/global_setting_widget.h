#pragma once

#include <agz/editor/common.h>

#include "ui_GlobalSettingWidget.h"

AGZ_EDITOR_BEGIN

class GlobalSettingWidget : public QWidget, public Ui::GlobalSettingWidget
{
public:

    explicit GlobalSettingWidget(QWidget *parent);
};

AGZ_EDITOR_END
