#pragma once

#include <QCheckBox>
#include <QComboBox>

#include <agz/editor/ui/transform2d_widget.h>
#include <agz/editor/ui/utility/combobox_without_wheel.h>

AGZ_EDITOR_BEGIN

class AssetLoader;
class AssetSaver;

class Texture2DCommonParamsWidget : public QWidget
{
    Q_OBJECT

public:

    struct InitData
    {
        bool apply_inv_gamma = false;
        real inv_gamma       = real(2.2);

        bool inv_u   = false;
        bool inv_v   = false;
        bool swap_uv = false;

        Transform2DWidget *transform = nullptr;

        QString wrap_u = "Clamp";
        QString wrap_v = "Clamp";
    };

    explicit Texture2DCommonParamsWidget(const InitData &init_data = {});

    Texture2DCommonParamsWidget *clone() const;

    tracer::Texture2DCommonParams get_tracer_params() const;

    void save_asset(AssetSaver &saver) const;

    void load_asset(AssetLoader &loader);

    void to_config(tracer::ConfigGroup &grp)  const;

signals:

    void change_params();

private:

    QCheckBox *apply_inv_gamma_ = nullptr;
    QLineEdit *inv_gamma_       = nullptr;

    Box<QValidator> inv_gamma_validator_;

    QCheckBox *inv_u_ = nullptr;
    QCheckBox *inv_v_ = nullptr;
    QCheckBox *swap_uv_ = nullptr;

    Transform2DWidget *transform_ = nullptr;

    ComboBoxWithoutWheelFocus *wrap_u_ = nullptr;
    ComboBoxWithoutWheelFocus *wrap_v_ = nullptr;
};

AGZ_EDITOR_END
