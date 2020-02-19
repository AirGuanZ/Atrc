#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include <im3d.h>

#include <agz/editor/common.h>

namespace Im3d
{

class Im3dInst : public agz::misc::uncopyable_t
{
public:

    // must be called with a gl context
    Im3dInst(int width, int height);

    // must be called with a gl context
    ~Im3dInst();

    void update(const Vec2 &cursor_pos, bool left_button_down);

    void new_frame();

    // must be called with a gl context
    void draw(QOpenGLFunctions *gl);

    void set_framebuffer(int width, int height);

    void set_camera(
        const Vec3 &pos, const Vec3 &dir, const Vec3 &up, float fov_deg);

private:

    void init_im3d();

    void destroy_im3d();

    int fb_width_;
    int fb_height_;

    Vec3 cam_pos_;
    Vec3 cam_dir_;
    Vec3 cam_up_;
    float cam_fov_deg_;

    Mat4 cam_world_;
    Mat4 cam_view_;
    Mat4 cam_proj_;
    Mat4 cam_proj_view_;

    Vec2 curr_cursor_pos_;

    // rendering

    QOpenGLShaderProgram shader_points_;
    QOpenGLShaderProgram shader_lines_;
    QOpenGLShaderProgram shader_triangles_;
};

} // namespace Im3d
