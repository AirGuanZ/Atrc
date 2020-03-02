#pragma once

#include <QCheckBox>

#include <agz/editor/texture2d/texture2d.h>

AGZ_EDITOR_BEGIN

class NormalMapWidget : public QWidget
{
    Q_OBJECT

public:

    struct InitData
    {
        bool apply_normal_map = false;
        Texture2DSlot *normal_map = nullptr;
    };

    NormalMapWidget(const InitData &init_data, ObjectContext &obj_ctx);

    NormalMapWidget *clone() const;

    void save_asset(AssetSaver &saver);

    void load_asset(AssetLoader &loader);

    std::unique_ptr<tracer::NormalMapper> get_tracer_object() const;

signals:

    void change_params();

private:

    ObjectContext &obj_ctx_;

    QCheckBox *apply_normal_map_ = nullptr;

    Texture2DSlot *normal_map_ = nullptr;
};

AGZ_EDITOR_END
