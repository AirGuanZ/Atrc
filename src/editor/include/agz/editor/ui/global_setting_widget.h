#pragma once

#include <agz/editor/common.h>

#include "ui_GlobalSettingWidget.h"

AGZ_EDITOR_BEGIN

class AssetLoader;
class AssetSaver;

class GlobalSettingWidget : public QWidget, public Ui::GlobalSettingWidget
{
public:

    explicit GlobalSettingWidget(QWidget *parent);

    void save_asset(AssetSaver &saver) const;

    void load_asset(AssetLoader &loader);
};

AGZ_EDITOR_END
