#pragma once

#include <QPushButton>
#include <QWidget>

#include <agz/editor/ui/utility/vec_input.h>

AGZ_EDITOR_BEGIN

class AssetLoader;
class AssetSaver;

/**
 * there is two groups of camera params: PREVIEW & RENDER
 *
 * PREVIEW can be edited directly or modified by PreviewWindow
 * RENDER has three possible state:
 *    1. EDIT MODE: can be edited directly
 *    2. BIND MODE: can be bounded to PREVIEW
 *
 * thus the RENDER section provides two buttons:
 *    1. Set PREVIEW to RENDER
 *        PREVIEW.pos/dst/up = RENDER.pos/dst/up
          adjust PREVIEW.fov to make sure that
            PREVIEW.viewport contains RENDER.viewport
 *    2. Bind RENDER to PREVIEW (Checkable)
          adjust PREVIEW.fov to make sure that
            PREVIEW.viewport contains RENDER.viewport
 *        keep RENDER.pos/dst/up = PREVIEW.pos/dst/up
 */
class CameraPanel : public QWidget
{
    Q_OBJECT

public:

    struct CameraParams
    {
        real distance = 1;
        Vec2 radian;
        Vec3 position;
        Vec3 look_at;
        Vec3 up;

        real fov_deg        = 60;
        real lens_radius    = 0;
        real focal_distance = 1;

        Vec3 dir() const noexcept;
    };
    
    explicit CameraPanel(QWidget *parent = nullptr);

    void set_preview_aspect(real aspect) noexcept;

    const CameraParams &get_preview_params() const noexcept;

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

    void edit_distance(real new_distance);

    void edit_radian(const Vec2 &new_radian);

    void edit_position(const Vec3 &new_position);

    void edit_look_at(const Vec3 &new_look_at);

    void edit_up(const Vec3 &up);

    void edit_fov(real new_fov_deg);

    void edit_lens_radius(real new_lens_radius);

    void edit_focal_distance(real new_focal_distance);

    void edit_render_framesize();

    void click_set_render();

    void click_bind_render();

private:

    static Vec2 pos_to_radian(
        const Vec3 &pos, const Vec3 &dst, const Vec3 &up);

    static Vec3 radian_to_pos(
        const Vec2 &radian, const Vec3 &dst, real distance, const Vec3 up);

    void init_ui();

    void fetch_params_from_ui();

    real min_preview_fov_deg();

    real preview_aspect_ = 1;

    // preview section

    CameraParams preview_params_;

    struct PreviewWidgets
    {
        RealInput *distance = nullptr;
        Vec2Input *radian   = nullptr;
        Vec3Input *position = nullptr;
        Vec3Input *look_at  = nullptr;
        Vec3Input *up       = nullptr;

        RealInput *fov            = nullptr;
        RealInput *lens_radius    = nullptr;
        RealInput *focal_distance = nullptr;

    } preview_;

    // render section

    CameraParams export_params_;

    struct ExportWidgets : PreviewWidgets
    {
        QPushButton *display_render_camera = nullptr;
        QPushButton *bind_render_camera    = nullptr;

        QLineEdit *export_width  = nullptr;
        QLineEdit *export_height = nullptr;

    } render_;

    bool enable_export_frame_ = false;

    int export_width_  = 1024;
    int export_height_ = 768;
    Box<QIntValidator> export_framesize_validator_;
};

AGZ_EDITOR_END
