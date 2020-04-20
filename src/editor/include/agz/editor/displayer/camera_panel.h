#pragma once

#include <QPushButton>

#include <agz/editor/ui/utility/vec_input.h>

AGZ_EDITOR_BEGIN

class AssetLoader;
class AssetSaver;

class CameraParamsWidget : public QWidget
{
    Q_OBJECT

public:

    struct Params
    {
        real distance = 1;
        Vec2 radian;
        Vec3 position = Vec3(4, 0, 0);
        Vec3 look_at;
        Vec3 up = Vec3(0, 0, 1);

        real fov_deg        = 60;
        real lens_radius    = 0;
        real focal_distance = 1;

        Params();

        Vec3 dir() const noexcept;
    };

    CameraParamsWidget();

    const Params &get_params() const noexcept;

    void save_asset(AssetSaver &saver) const;

    void load_asset(AssetLoader &loader);

    void set_distance(real new_distance);

    void set_radian(const Vec2 &new_radian);

    void set_look_at(const Vec3 &new_look_at);

    void copy_pos_from(const Params &params);

signals:

    void edit_params();

private slots:

    void edit_distance(real new_dis);

    void edit_radian(const Vec2 &new_radian);

    void edit_position(const Vec3 &new_position);

    void edit_look_at(const Vec3 &look_at);

    void edit_up(const Vec3 &up);

    void edit_fov(real fov);

    void edit_lens_radius(real lens_radius);

    void edit_focal_distance(real focal_distance);

private:

    void update_ui_from_params();
    
    RealInput *distance_ = nullptr;
    Vec2Input *radian_   = nullptr;
    Vec3Input *position_ = nullptr;
    Vec3Input *look_at_  = nullptr;
    Vec3Input *up_       = nullptr;

    RealInput *fov_            = nullptr;
    RealInput *lens_radius_    = nullptr;
    RealInput *focal_distance_ = nullptr;

    Params params_;
};

class CameraPanel : public QWidget
{
    Q_OBJECT

public:

    using CameraParams = CameraParamsWidget::Params;

    explicit CameraPanel(QWidget *parent = nullptr);

    void set_display_aspect(real aspect) noexcept;

    const CameraParams &get_display_params() const noexcept;

    const CameraParams &get_export_params() const noexcept;

    int get_export_frame_width() const noexcept;

    int get_export_frame_height() const noexcept;

    bool is_export_frame_enabled() const noexcept;

    void set_distance(real new_distance);

    void set_radian(const Vec2 &new_radian);

    void set_look_at(const Vec3 &new_look_at);

    void save_asset(AssetSaver &saver) const;

    void load_asset(AssetLoader &loader);

signals:

    void edit_params();

private slots:

    void set_preview_to_export();

    void set_export_to_preview();

    void switch_editing_export();

    void change_export_framebuffer_size();

private:

    void update_display_params();

    real display_window_aspect_ = 1;

    // preview camera

    CameraParamsWidget *preview_;
    
    // exported camera

    CameraParamsWidget *export_;
    
    // displayed camera

    CameraParams display_params_;

    // others

    QPushButton *set_preview_to_export_ = nullptr;
    QPushButton *set_export_to_preview_ = nullptr;
    QPushButton *editing_export_        = nullptr;

    QLineEdit *export_width_  = nullptr;
    QLineEdit *export_height_ = nullptr;
    Box<QIntValidator> export_framesize_validator_;
};

AGZ_EDITOR_END
