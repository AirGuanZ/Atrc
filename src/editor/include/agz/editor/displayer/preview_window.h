#pragma once

#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QOpenGLVertexArrayObject>
#include <QPointer>

#include <agz/editor/displayer/camera_panel.h>
#include <agz/editor/displayer/gizmo_selector.h>
#include <agz/editor/displayer/im3d_inst.h>
#include <agz/editor/resource/entity_interface.h>
#include <agz/editor/utility/direct_transform3d.h>
#include <agz/tracer/utility/config.h>

AGZ_EDITOR_BEGIN

class Editor;

class PreviewWindow : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:

    PreviewWindow(QWidget *parent, Editor *editor);

    ~PreviewWindow();

    RC<tracer::Camera> create_camera();

    CameraPanel *get_camera_panel();

    // image preview

    void set_preview_image(const Image2D<Spectrum> &img);

    bool is_in_realtime_mode() const noexcept;

    void set_realtime_mode(bool realtime);

    // realtime mode

    using MeshID = int;

    using Vertex = EntityInterface::Vertex;

    MeshID generate_mesh_id() noexcept;

    void add_mesh(MeshID id, const Vertex *vertices, int vertex_count);

    void remove_mesh(MeshID id);

    void set_mesh_transform(MeshID id, const DirectTransform &world);

    void set_selected_mesh(MeshID id);

    // s/l

    void save_asset(AssetSaver &saver) const;

    void load_asset(AssetLoader &loader);

    RC<tracer::ConfigGroup> to_config() const;

signals:

    void left_click_to_emit_ray(const Vec3 &o, const Vec3 &d);

    void edit_render_mode();

    void edit_gizmo(const DirectTransform &new_transform);

protected:

    void initializeGL() override;

    void paintGL() override;

    void leaveEvent(QEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

private:

    void init_ui();

    void update_im3d_camera();

    void render_preview_image();

    void render_entities();

    void render_renderframe();

    void render_gizmo();

    void destroy();

    QPointer<CameraPanel> camera_panel_;

    // camera controller

    enum class CameraControllerState
    {
        Free, Rotate, Move
    };

    CameraControllerState camera_controller_state_ = CameraControllerState::Free;

    Vec2i press_coord_;
    Vec2 press_radian_;
    Vec3 press_dst_;

    const real rotate_speed_  = real(0.004);
    const real panning_speed_ = real(0.001);

    Editor *editor_;

    // mesh group

    struct Mesh
    {
        DirectTransform world;
        int vertex_count = 0;

        QOpenGLBuffer            vbo;
        QOpenGLVertexArrayObject vao;
    };

    std::unordered_map<MeshID, Mesh> id2mesh_;

    MeshID next_gen_mesh_id_ = 0;
    MeshID selected_mesh_id_ = -1;

    bool in_realtime_mode_ = false;
    QPushButton *always_highlight_selected_mesh_ = nullptr;

    // im3d

    bool im3d_cursor_moved_ = false;

    Vec2 cursor_pos_;
    bool is_left_button_just_down_ = false;
    bool is_left_button_down_      = false;

    GizmoSelector *gizmo_selector_ = nullptr;

    Box<Im3d::Im3dInst> im3d_;

    QMatrix4x4 proj_view_;

    // entity mesh rendering

    struct EntityShaderUniforms
    {
        int world     = 0;
        int mvp       = 0;
        int color     = 0;
        int light_dir = 0;
    };

    EntityShaderUniforms entity_uniforms_;

    Box<QOpenGLShaderProgram> entity_shader_;

    // preview image rendering

    struct PreviewShaderUniforms
    {
        int tex = 0;
    };

    PreviewShaderUniforms preview_uniforms_;

    Box<QOpenGLShaderProgram> preview_shader_;

    Box<QOpenGLTexture> preview_tex_;

    QOpenGLBuffer            preview_vbo_;
    QOpenGLVertexArrayObject preview_vao_;

    // render frame rendering

    struct RenderFrameShaderUniforms
    {
        int world = 0;
    };

    RenderFrameShaderUniforms render_frame_uniforms_;

    Box<QOpenGLShaderProgram> render_frame_shader_;

    QOpenGLBuffer            render_frame_vbo_;
    QOpenGLVertexArrayObject render_frame_vao_;
};

AGZ_EDITOR_END
