#pragma once

#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QOpenGLVertexArrayObject>

#include <agz/editor/displayer/gizmo_selector.h>
#include <agz/editor/displayer/im3d_inst.h>
#include <agz/editor/resource/entity_interface.h>
#include <agz/editor/utility/direct_transform3d.h>

AGZ_EDITOR_BEGIN

class DisplayerGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:

    using Vertex = EntityInterface::Vertex;

    using MeshID = int;

    DisplayerGLWidget();

    ~DisplayerGLWidget();

    MeshID generate_mesh_id();

    void add_mesh(MeshID id, const Vertex *vertices, int vertex_count);

    void remove_mesh(MeshID id);

    void set_transform(MeshID id, const DirectTransform &world);

    void set_camera(
        const QMatrix4x4 &proj_view, const Vec3 &cam_pos, const Vec3 &cam_dir,
        const Vec3 &cam_up, real cam_fov_deg);

    void set_selected_mesh(MeshID id);

    void set_background_image(const Image2D<Spectrum> &img);

    bool is_in_realtime_mode() const;

    void cursor_leave(QEvent *event);

    void mouse_move(QMouseEvent *event);

    void mouse_press(QMouseEvent *event);

    void mouse_release(QMouseEvent *event);

signals:

    // left button clicked at film_coord
    void free_left_click(const Vec2 &film_coord);

    // switch between realtime mode/offline mode
    void switch_render_mode();

    // gizmo is directly edited
    void edit_gizmo(const DirectTransform &new_transform);

protected:

    void initializeGL() override;

    void paintGL() override;

private:

    void render_background_image();

    void render_entities();

    void render_gizmo();

    void destroy();

    // mesh group

    struct Mesh
    {
        int vertex_count = 0;

        DirectTransform world;

        QOpenGLBuffer            vbo;
        QOpenGLVertexArrayObject vao;
    };

    std::unordered_map<MeshID, Mesh> id2mesh_;

    bool in_realtime_mode_ = true;
    MeshID selected_mesh_id_ = -1;

    QPushButton *always_highlight_selected_ = nullptr;

    // camera params

    Vec3 cam_pos_;
    Vec3 cam_dir_;
    Vec3 cam_up_;
    real cam_fov_deg_ = 60;
    QMatrix4x4 proj_view_;

    // im3d

    bool cursor_moved_ = false;

    GizmoSelector *gizmo_selector_ = nullptr;

    std::unique_ptr<Im3d::Im3dInst> im3d_;

    // entity mesh rendering

    std::unique_ptr<QOpenGLShaderProgram> entity_shader_;

    struct EntityShaderUniforms
    {
        int world_loc_     = 0;
        int mvp_loc_       = 0;
        int color_loc_     = 0;
        int light_dir_loc_ = 0;

    } entity_uniforms_;

    // background image rendering

    std::unique_ptr<QOpenGLShaderProgram> background_shader_;

    struct BackgroundShaderUniforms
    {
        int background_tex_loc_ = 0;

    } background_uniforms_;

    std::unique_ptr<QOpenGLTexture> background_tex_;

    QOpenGLVertexArrayObject background_vao_;
    QOpenGLBuffer            background_vbo_;

    MeshID next_gen_mesh_id_ = 0;

    // cursor tracker

    Vec2 cursor_pos_;
    bool is_left_button_just_down_ = false;
    bool is_left_button_down_      = false;
};

AGZ_EDITOR_END
