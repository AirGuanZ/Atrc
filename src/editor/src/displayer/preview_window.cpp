#include <QMouseEvent>
#include <QOpenGLDebugLogger>

#include <agz/editor/displayer/preview_window.h>
#include <agz/editor/editor.h>

AGZ_EDITOR_BEGIN

namespace
{

    const char *ENTITY_VERTEX_SHADER_SOURCE = R"___(
    #version 330 core

    uniform mat4 world;
    uniform mat4 proj_view_world;

    in vec3 position;
    in vec3 normal;

    out vec3 w_normal;

    void main()
    {
        w_normal = normalize((world * vec4(normal, 0)).xyz);
        gl_Position = proj_view_world * vec4(position, 1);
    }
)___";

    const char *ENTITY_FRAGMENT_SHADER_SOURCE = R"___(
    #version 330 core

    uniform vec3 color;
    uniform vec3 light_dir;

    in vec3 w_normal;

    out vec4 frag_color;

    void main()
    {
        if(light_dir.x == 0 && light_dir.y == 0 && light_dir.z == 0)
        {
            frag_color = vec4(color, 0.2);
            return;
        }
        float light_factor = 0.1 + 0.75 * max(0, dot(normalize(w_normal), -light_dir));
        light_factor = 0.4 * light_factor + 0.6 * light_factor * light_factor;
        light_factor = pow(light_factor, 1 / 1.4);
        frag_color = vec4(light_factor * color, 1);
    }
)___";

    const char *PREVIEW_VERTEX_SHADER_SOURCE = R"___(
    #version 330 core

    in vec2 position;
    in vec2 texcoord;

    out vec2 uv;

    void main()
    {
        uv = texcoord;
        gl_Position = vec4(position, 0.5, 1);
    }
)___";

    const char *PREVIEW_FRAGMENT_SHADER_SOURCE = R"___(
    #version 330 core

    uniform sampler2D tex;

    in vec2 uv;

    out vec4 frag_color;

    void main()
    {
        vec3 color = texture(tex, uv).rgb;
        frag_color = vec4(pow(color, vec3(1 / 2.2)), 1);
    }
)___";

    const char *RENDER_FRAME_VERTEX_SHADER_SOURCE = R"___(
    #version 330 core

    uniform mat3 world;

    in vec2 position;

    void main()
    {
        gl_Position = vec4((world * vec3(position, 1)).xy, 0.5, 1);
    }
)___";

    const char *RENDER_FRAME_FRAGMENT_SHADER_SOURCE = R"___(
    #version 330 core

    out vec4 frag_color;

    void main()
    {
        frag_color = vec4(1, 0, 1, 1);
    }

)___";

} // namespace anonymous

PreviewWindow::PreviewWindow(QWidget *parent, Editor *editor)
    : QOpenGLWidget(parent), editor_(editor)
{
    init_ui();
}

PreviewWindow::~PreviewWindow()
{
    destroy();
}

void PreviewWindow::init_ui()
{
    QHBoxLayout *layout             = new QHBoxLayout(this);
    QPushButton *switch_mode        = new QPushButton("Switch Mode", this);
    always_highlight_selected_mesh_ = new QPushButton("H", this);
    gizmo_selector_                 = new GizmoSelector;

    switch_mode->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    always_highlight_selected_mesh_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    always_highlight_selected_mesh_->setCheckable(true);
    always_highlight_selected_mesh_->setChecked(false);

    layout->addWidget(switch_mode);
    layout->addWidget(gizmo_selector_);
    layout->addWidget(always_highlight_selected_mesh_);
    layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    connect(switch_mode, &QPushButton::clicked, [=]
    {
        in_realtime_mode_ = !in_realtime_mode_;
        emit edit_render_mode();
        update();
    });

    connect(always_highlight_selected_mesh_, &QPushButton::clicked, [=]
    {
        update();
    });

    connect(gizmo_selector_, &GizmoSelector::change_gizmo, [=]
    {
        update();
    });

    QSurfaceFormat format;
    format.setMajorVersion(3);
    format.setMinorVersion(3);
    format.setProfile(QSurfaceFormat::CoreProfile);
#ifdef AGZ_DEBUG
    format.setOption(QSurfaceFormat::DebugContext);
#endif
    setFormat(format);

    setMouseTracking(true);

    update();
}

std::shared_ptr<tracer::Camera> PreviewWindow::create_camera()
{
    const auto &camera_params = camera_panel_->get_preview_params();

    const int film_width = size().width();
    const int film_height = size().height();
    const real film_aspect = static_cast<real>(film_width) / film_height;

    const auto up_coord = tracer::Coord::from_z(camera_params.up);
    const Vec3 local_dir = {
        std::sin(camera_params.radian.y) * std::cos(camera_params.radian.x),
        std::sin(camera_params.radian.y) * std::sin(camera_params.radian.x),
        std::cos(camera_params.radian.y)
    };
    const Vec3 global_dir = up_coord.local_to_global(local_dir);
    const Vec3 pos = camera_params.look_at + global_dir * camera_params.distance;

    return tracer::create_thin_lens_camera(
        film_aspect, pos, camera_params.look_at, camera_params.up,
        math::deg2rad(real(camera_params.fov_deg)),
        real(camera_params.lens_radius),
        real(camera_params.focal_distance));
}

CameraPanel *PreviewWindow::get_camera_panel()
{
    if(camera_panel_)
        return camera_panel_;

    camera_panel_ = new CameraPanel(this);

    connect(camera_panel_, &CameraPanel::edit_params, [=]
    {
        update_im3d_camera();
        update();
        editor_->on_change_camera();
    });

    return camera_panel_;
}

void PreviewWindow::set_preview_image(const Image2D<Spectrum> &img)
{
    if(!context())
        return;

    makeCurrent();

    if(img.width() > 0 && img.height() > 0)
    {
        preview_tex_ = std::make_unique<QOpenGLTexture>(QOpenGLTexture::Target2D);
        preview_tex_->create();
        preview_tex_->setSize(img.width(), img.height());
        preview_tex_->setFormat(QOpenGLTexture::RGB16F);
        preview_tex_->setMipLevels(1);
        preview_tex_->allocateStorage(QOpenGLTexture::RGB, QOpenGLTexture::Float16);
        preview_tex_->setData(
            QOpenGLTexture::RGB, QOpenGLTexture::Float32, img.raw_data());
        preview_tex_->setMinMagFilters(
            QOpenGLTexture::Nearest, QOpenGLTexture::Nearest);
    }
    else
        preview_tex_.reset();

    doneCurrent();
    update();
}

bool PreviewWindow::is_in_realtime_mode() const noexcept
{
    return in_realtime_mode_;
}

PreviewWindow::MeshID PreviewWindow::generate_mesh_id() noexcept
{
    return next_gen_mesh_id_++;
}

void PreviewWindow::add_mesh(MeshID id, const Vertex *vertices, int vertex_count)
{
    makeCurrent();

    assert(vertices && vertex_count > 0 && vertex_count % 3 == 0);
    remove_mesh(id);

    Mesh &mesh = id2mesh_[id];

    mesh.world = DirectTransform();

    {
        mesh.vao.create();
        mesh.vao.bind();
        
        mesh.vbo.create();
        mesh.vbo.bind();
        mesh.vbo.allocate(vertices, sizeof(Vertex) * vertex_count);
        mesh.vertex_count = vertex_count;

        auto gl = QOpenGLContext::currentContext()->functions();
        gl->glEnableVertexAttribArray(0);
        gl->glEnableVertexAttribArray(1);
        gl->glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), nullptr);
        gl->glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, nor)));

        mesh.vbo.release();
        mesh.vao.release();
    }

    doneCurrent();

    AGZ_INFO("add mesh with id: {}, vertex count: {}", id, vertex_count);

    update();
}

void PreviewWindow::remove_mesh(MeshID id)
{
    const auto it = id2mesh_.find(id);
    if(it == id2mesh_.end())
        return;

    it->second.vbo.destroy();
    it->second.vao.destroy();
    id2mesh_.erase(it);

    AGZ_INFO("remove mesh with id: {}", id);

    update();
}

void PreviewWindow::set_mesh_transform(MeshID id, const DirectTransform &world)
{
    const auto it = id2mesh_.find(id);
    assert(it != id2mesh_.end());
    it->second.world = world;

    update();
}

void PreviewWindow::set_selected_mesh(MeshID id)
{
    selected_mesh_id_ = id;
    update();
}

void PreviewWindow::save_asset(AssetSaver &saver) const
{
    assert(camera_panel_);
    camera_panel_->save_asset(saver);
}

void PreviewWindow::load_asset(AssetLoader &loader)
{
    get_camera_panel()->load_asset(loader);
    update_im3d_camera();
}

std::shared_ptr<tracer::ConfigGroup> PreviewWindow::to_config() const
{
    assert(camera_panel_);
    const auto &params = camera_panel_->get_render_params();

    auto grp = std::make_shared<tracer::ConfigGroup>();
    grp->insert_str("type", "thin_lens");
    grp->insert_child("pos", tracer::ConfigArray::from_vec3(params.position));
    grp->insert_child("dst", tracer::ConfigArray::from_vec3(params.look_at));
    grp->insert_child("up", tracer::ConfigArray::from_vec3(params.up));
    grp->insert_real("fov", params.fov_deg);
    grp->insert_real("lens_radius", params.lens_radius);
    grp->insert_real("focal_distance", params.focal_distance);

    return grp;
}

void PreviewWindow::initializeGL()
{
    // debugger

#ifdef AGZ_DEBUG
    QOpenGLDebugLogger *logger = new QOpenGLDebugLogger(this);
    connect(logger, &QOpenGLDebugLogger::messageLogged, [=](const QOpenGLDebugMessage &msg)
    {
        std::cerr << msg.message().toStdString();
    });
    logger->initialize();
    logger->startLogging();
#endif

    // opengl funcs

    initializeOpenGLFunctions();

    // entity shader

    entity_shader_ = std::make_unique<QOpenGLShaderProgram>();
    entity_shader_->addShaderFromSourceCode(QOpenGLShader::Vertex,   ENTITY_VERTEX_SHADER_SOURCE);
    entity_shader_->addShaderFromSourceCode(QOpenGLShader::Fragment, ENTITY_FRAGMENT_SHADER_SOURCE);
    entity_shader_->link();

    entity_shader_->bindAttributeLocation("position", 0);
    entity_shader_->bindAttributeLocation("normal", 1);

    entity_shader_->bind();
    entity_uniforms_.world     = entity_shader_->uniformLocation("world");
    entity_uniforms_.mvp       = entity_shader_->uniformLocation("proj_view_world");
    entity_uniforms_.color     = entity_shader_->uniformLocation("color");
    entity_uniforms_.light_dir = entity_shader_->uniformLocation("light_dir");
    entity_shader_->release();

    // background shader

    preview_shader_ = std::make_unique<QOpenGLShaderProgram>();
    preview_shader_->addShaderFromSourceCode(QOpenGLShader::Vertex,   PREVIEW_VERTEX_SHADER_SOURCE);
    preview_shader_->addShaderFromSourceCode(QOpenGLShader::Fragment, PREVIEW_FRAGMENT_SHADER_SOURCE);
    preview_shader_->link();

    preview_shader_->bindAttributeLocation("position", 0);
    preview_shader_->bindAttributeLocation("texcoord", 1);

    preview_shader_->bind();
    preview_uniforms_.tex = preview_shader_->uniformLocation("tex");
    preview_shader_->release();

    // background buffer

    preview_vao_.create();
    preview_vao_.bind();

    preview_vbo_.create();
    preview_vbo_.bind();

    struct BackgroundVertex
    {
        Vec2 position;
        Vec2 texcoord;
    };

    const BackgroundVertex background_vertices[] =
    {
        { { -1, -1 }, { 0, 0 } },
        { { -1, +1 }, { 0, 1 } },
        { { +1, +1 }, { 1, 1 } },
        { { -1, -1 }, { 0, 0 } },
        { { +1, +1 }, { 1, 1 } },
        { { +1, -1 }, { 1, 0 } }
    };
    preview_vbo_.allocate(background_vertices, sizeof(background_vertices));

    auto gl = QOpenGLContext::currentContext()->functions();
    gl->glEnableVertexAttribArray(0);
    gl->glEnableVertexAttribArray(1);
    gl->glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE,
        sizeof(BackgroundVertex),
        reinterpret_cast<void*>(offsetof(BackgroundVertex, position)));
    gl->glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE,
        sizeof(BackgroundVertex),
        reinterpret_cast<void*>(offsetof(BackgroundVertex, texcoord)));

    preview_vbo_.release();
    preview_vao_.release();

    // render frame shader
    
    render_frame_shader_ = std::make_unique<QOpenGLShaderProgram>();
    render_frame_shader_->addShaderFromSourceCode(QOpenGLShader::Vertex,   RENDER_FRAME_VERTEX_SHADER_SOURCE);
    render_frame_shader_->addShaderFromSourceCode(QOpenGLShader::Fragment, RENDER_FRAME_FRAGMENT_SHADER_SOURCE);
    render_frame_shader_->link();

    render_frame_shader_->bindAttributeLocation("position", 0);

    render_frame_shader_->bind();
    render_frame_uniforms_.world = render_frame_shader_->uniformLocation("world");
    render_frame_shader_->release();

    // render frame buffer
    
    render_frame_vao_.create();
    render_frame_vao_.bind();

    render_frame_vbo_.create();
    render_frame_vbo_.bind();

    struct RenderFrameVertex
    {
        Vec2 position;
    };

    const RenderFrameVertex render_frame_vertices[] =
    {
        { { 0, 0 } }, { { 0, 1 } },
        { { 0, 1 } }, { { 1, 1 } },
        { { 1, 1 } }, { { 1, 0 } },
        { { 1, 0 } }, { { 0, 0 } }
    };
    render_frame_vbo_.allocate(render_frame_vertices, sizeof(render_frame_vertices));

    gl->glEnableVertexAttribArray(0);
    gl->glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE,
        sizeof(RenderFrameVertex),
        reinterpret_cast<void*>(offsetof(RenderFrameVertex, position)));

    render_frame_vbo_.release();
    render_frame_vao_.release();

    // im3d

    const auto &cam = get_camera_panel()->get_preview_params();
    const Vec3 cam_pos = cam.position;
    const Vec3 cam_dir = cam.dir();
    const Vec3 cam_up  = cam.up;

    im3d_ = std::make_unique<Im3d::Im3dInst>(width(), height());
    im3d_->set_camera(
        { cam_pos.x, cam_pos.y, cam_pos.z },
        { cam_dir.x, cam_dir.y, cam_dir.z },
        { cam_up.x, cam_up.y, cam_up.z },
        cam.fov_deg);

    im3d_->set_framebuffer(width(), height());
    im3d_->update({ 0, 0 }, false);
}

void PreviewWindow::paintGL()
{
    im3d_->set_framebuffer(width(), height());
    im3d_->update({ cursor_pos_.x, cursor_pos_.y }, is_left_button_down_);
    im3d_->new_frame();

    if(!in_realtime_mode_ && preview_tex_)
        render_preview_image();

    if(in_realtime_mode_)
        render_entities();

    render_gizmo();

    render_renderframe();

    makeCurrent();

    glDisable(GL_DEPTH_TEST);
    im3d_->draw(QOpenGLContext::currentContext()->functions());
}

void PreviewWindow::leaveEvent(QEvent *event)
{
    camera_controller_state_ = CameraControllerState::Free;
    is_left_button_down_ = false;
    update();
}

void PreviewWindow::resizeEvent(QResizeEvent *event)
{
    QOpenGLWidget::resizeEvent(event);

    update_im3d_camera();
    editor_->on_change_camera();

    get_camera_panel()->set_preview_aspect(real(event->size().width()) / event->size().height());
}

void PreviewWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MouseButton::LeftButton)
    {
        is_left_button_just_down_ = true;
        is_left_button_down_ = true;
        cursor_pos_ = { float(event->x()), float(event->y()) };

        update();
    }

    if(event->button() == Qt::MouseButton::RightButton)
    {
        camera_controller_state_ = CameraControllerState::Rotate;
        press_coord_ = { event->x(), event->y() };
        press_radian_ = camera_panel_->get_preview_params().radian;
        press_dst_ = camera_panel_->get_preview_params().look_at;
        return;
    }

    if(event->button() == Qt::MouseButton::MiddleButton)
    {
        camera_controller_state_ = CameraControllerState::Move;
        press_coord_ = { event->x(), event->y() };
        press_radian_ = camera_panel_->get_preview_params().radian;
        press_dst_ = camera_panel_->get_preview_params().look_at;
    }
}

void PreviewWindow::mouseMoveEvent(QMouseEvent *event)
{
    cursor_pos_ = { float(event->x()), float(event->y()) };
    im3d_cursor_moved_ = true;
    update();

    if(camera_controller_state_ == CameraControllerState::Rotate)
    {
        const int dx = event->x() - press_coord_.x;
        const int dy = event->y() - press_coord_.y;

        if(!dx && !dy)
            return;

        const real new_rad_x = press_radian_.x - rotate_speed_ * real(dx);
        const real new_rad_y = math::clamp<real>(
            press_radian_.y - rotate_speed_ * dy, real(0.001), PI_r - real(0.001));

        camera_panel_->set_radian({ new_rad_x, new_rad_y });

        update_im3d_camera();
        editor_->on_change_camera();
        return;
    }

    if(camera_controller_state_ == CameraControllerState::Move)
    {
        const int dx = event->x() - press_coord_.x;
        const int dy = event->y() - press_coord_.y;

        if(!dx && !dy)
            return;

        const Vec3 dst_to_pos = -camera_panel_->get_preview_params().dir();

        const Vec3 ex = -cross(camera_panel_->get_preview_params().up, dst_to_pos).normalize();
        const Vec3 ey = -cross(dst_to_pos, ex).normalize().normalize();

        const Vec3 new_look_at = press_dst_ + panning_speed_ * camera_panel_->get_preview_params().distance
                                                             * (real(dx) * ex + real(dy) * ey);
        camera_panel_->set_look_at(new_look_at);

        update_im3d_camera();
        editor_->on_change_camera();
    }
}

void PreviewWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MouseButton::LeftButton)
    {
        is_left_button_down_ = false;
        cursor_pos_ = { float(event->x()), float(event->y()) };
        update();
    }

    if(camera_controller_state_ == CameraControllerState::Move &&
       event->button() == Qt::MouseButton::MiddleButton)
        camera_controller_state_ = CameraControllerState::Free;

    if(camera_controller_state_ == CameraControllerState::Rotate &&
       event->button() == Qt::MouseButton::RightButton)
        camera_controller_state_ = CameraControllerState::Free;
}

void PreviewWindow::wheelEvent(QWheelEvent *event)
{
    real distance = camera_panel_->get_preview_params().distance;
    const real delta = real(0.05) * distance;

    distance -= delta * event->angleDelta().y() / real(120);
    distance = (std::max)(distance, real(0.01));

    camera_panel_->set_distance(distance);

    update_im3d_camera();
    editor_->on_change_camera();
}

void PreviewWindow::update_im3d_camera()
{
    const real aspect = real(width()) / height();
    const auto &params = get_camera_panel()->get_preview_params();

    QMatrix4x4 proj;
    proj.perspective(params.fov_deg, aspect, real(0.1), real(1000));

    QMatrix4x4 view;
    view.lookAt(
        { params.position.x, params.position.y, params.position.z },
        { params.look_at.x, params.look_at.y, params.look_at.z },
        { params.up.x, params.up.y, params.up.z });

    proj_view_ = proj * view;

    if(im3d_)
    {
        const Vec3 cam_dir = params.dir();

        im3d_->set_camera(
            { params.position.x, params.position.y, params.position.z },
            { cam_dir.x, cam_dir.y, cam_dir.z },
            { params.up.x, params.up.y, params.up.z },
            params.fov_deg);
    }

    update();
}

void PreviewWindow::render_preview_image()
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    preview_shader_->bind();
    preview_vao_.bind();
    preview_tex_->bind(0);

    preview_shader_->setUniformValue(preview_uniforms_.tex, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    preview_tex_->release(0);
    preview_vao_.release();
    preview_shader_->release();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    entity_shader_->bind();

    if(always_highlight_selected_mesh_->isChecked() && selected_mesh_id_ >= 0)
    {
        const Vec3 cam_dir = get_camera_panel()->get_preview_params().dir();

        const real cam_hori_rad = tracer::local_angle::phi(cam_dir);
        const real cam_vert_rad = tracer::local_angle::theta(cam_dir);

        const QVector3D light_dir = QVector3D(
            std::sin(cam_vert_rad + 0.3f) * std::cos(cam_hori_rad - 0.2f),
            std::sin(cam_vert_rad + 0.3f) * std::sin(cam_hori_rad - 0.2f),
            std::cos(cam_vert_rad + 0.3f)).normalized();

        auto it = id2mesh_.find(selected_mesh_id_);
        assert(it != id2mesh_.end());
        auto &mesh = it->second;

        mesh.vao.bind();

        const Mat4 world_t = mesh.world.compose().transpose();

        const QMatrix4x4 world(&world_t.data[0][0]);
        const QMatrix4x4 mvp = proj_view_ * world;

        entity_shader_->setUniformValue(entity_uniforms_.world, world);
        entity_shader_->setUniformValue(entity_uniforms_.mvp, mvp);
        entity_shader_->setUniformValue(entity_uniforms_.color, QVector3D(0, 1, 1));
        entity_shader_->setUniformValue(entity_uniforms_.light_dir, light_dir);

        glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_count);

        mesh.vao.release();
    }

    entity_shader_->release();
}

void PreviewWindow::render_entities()
{
    const Vec3 cam_dir = get_camera_panel()->get_preview_params().dir();

    const real cam_hori_rad = tracer::local_angle::phi(cam_dir);
    const real cam_vert_rad = tracer::local_angle::theta(cam_dir);

    const QVector3D light_dir = QVector3D(
        std::sin(cam_vert_rad + 0.3f) * std::cos(cam_hori_rad - 0.2f),
        std::sin(cam_vert_rad + 0.3f) * std::sin(cam_hori_rad - 0.2f),
        std::cos(cam_vert_rad + 0.3f)).normalized();

    auto render_mesh = [&](Mesh &mesh, const QVector3D &color, const QVector3D &light_d)
    {
        mesh.vao.bind();

        const Mat4 world_t = mesh.world.compose().transpose();

        const QMatrix4x4 world(&world_t.data[0][0]);
        const QMatrix4x4 mvp = proj_view_ * world;

        entity_shader_->setUniformValue(entity_uniforms_.world, world);
        entity_shader_->setUniformValue(entity_uniforms_.mvp, mvp);
        entity_shader_->setUniformValue(entity_uniforms_.color, color);
        entity_shader_->setUniformValue(entity_uniforms_.light_dir, light_d);

        glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_count);

        mesh.vao.release();
    };

    entity_shader_->bind();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(auto &p : id2mesh_)
    {
        if(p.first != selected_mesh_id_)
            render_mesh(p.second, { 0.7f, 0.7f, 0.7f }, light_dir);
    }

    if(selected_mesh_id_ >= 0)
    {
        auto it = id2mesh_.find(selected_mesh_id_);
        assert(it != id2mesh_.end());

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        glEnable(GL_BLEND);
        render_mesh(it->second, { 0.5f, 0, 0 }, QVector3D(0, 0, 0));
        glDisable(GL_BLEND);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        render_mesh(it->second, { 0, 1, 1 }, light_dir);
    }

    entity_shader_->release();
}

void PreviewWindow::render_renderframe()
{
    if(!get_camera_panel()->is_render_frame_enabled())
        return;

    const auto &preview_cam = camera_panel_->get_preview_params();
    const auto &render_cam  = camera_panel_->get_render_params();

    const real preview_aspect = real(width()) / height();
    const real render_aspect  = real(camera_panel_->get_render_frame_width()) / camera_panel_->get_render_frame_height();

    const real H = 2 * std::tan(math::deg2rad(preview_cam.fov_deg / 2));
    const real W = preview_aspect * H;

    const real h = 2 * std::tan(math::deg2rad(render_cam.fov_deg / 2));
    const real w = render_aspect * h;

    const Vec2 LB = { -w / W, -h / H };
    const Vec2 RT = { +w / W, +h / H };

    const Mat3 world = (Mat3::translate(LB) * Mat3::scale(RT - LB)).transpose();
    const QMatrix3x3 qworld(&world[0][0]);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    render_frame_shader_->bind();
    render_frame_vao_.bind();

    render_frame_shader_->setUniformValue(render_frame_uniforms_.world, qworld);

    glDrawArrays(GL_LINES, 0, 8);

    render_frame_vao_.release();
    render_frame_shader_->release();
}

void PreviewWindow::render_gizmo()
{
    AGZ_SCOPE_GUARD({
        if(is_left_button_just_down_)
        {
            is_left_button_just_down_ = false;
            if(Im3d::GetContext().m_hotDepth == (std::numeric_limits<float>::max)())
            {
                const Vec2 film_coord(
                    cursor_pos_.x     / width(),
                    1 - cursor_pos_.y / height());

                const auto &camera_params = get_camera_panel()->get_preview_params();

                const int film_width = size().width();
                const int film_height = size().height();
                const real film_aspect = static_cast<real>(film_width) / film_height;

                const Vec3 d = camera_params.dir();
                const Vec3 x = cross(d, camera_params.up).normalize();
                const Vec3 y = cross(x, d).normalize();

                const real y_ext = 2 * camera_params.distance * std::tan(
                    math::deg2rad(camera_params.fov_deg / 2));
                const real x_ext = film_aspect * y_ext;

                const Vec3 ray_dst = camera_params.look_at
                    + (film_coord.x - real(0.5)) * x_ext * x
                    + (film_coord.y - real(0.5)) * y_ext * y;

                const Vec3 eye = camera_panel_->get_preview_params().position;
                emit left_click_to_emit_ray(eye, ray_dst - eye);
            }
        }
        });

    if(selected_mesh_id_ < 0)
        return;

    const auto gizmo_type = gizmo_selector_->get_gizmo_type();
    if(gizmo_type == GizmoSelector::GizmoType::None)
        return;

    auto it = id2mesh_.find(selected_mesh_id_);
    if(it == id2mesh_.end())
        return;
    const MeshID id = it->first;
    Mesh &mesh = it->second;

    const Mat4 world = mesh.world.compose();

    Im3d::Mat4 im3d_world;
    std::memcpy(&im3d_world, &world, sizeof(Mat4));
    PushMatrix(im3d_world);

    bool is_edited;

    if(gizmo_type == GizmoSelector::GizmoType::Translate)
        is_edited = Im3d::GizmoTranslation(id, &mesh.world.translate[0], gizmo_selector_->is_local());
    else if(gizmo_type == GizmoSelector::GizmoType::Rotate)
        is_edited = Im3d::GizmoRotation(id, &mesh.world.rotate.data[0][0], gizmo_selector_->is_local());
    else
    {
        float scale[3] = { mesh.world.scale, mesh.world.scale, mesh.world.scale };
        is_edited = Im3d::GizmoScale(id, scale);
        if(is_edited)
        {
            if(mesh.world.scale != scale[0]) mesh.world.scale = scale[0];
            else if(mesh.world.scale != scale[1]) mesh.world.scale = scale[1];
            else if(mesh.world.scale != scale[2]) mesh.world.scale = scale[2];
        }
    }

    is_edited &= im3d_cursor_moved_;
    im3d_cursor_moved_ = false;

    Im3d::PopMatrix();

    if(is_edited)
        emit edit_gizmo(mesh.world);
}

void PreviewWindow::destroy()
{
    makeCurrent();

    for(auto &p : id2mesh_)
    {
        p.second.vao.destroy();
        p.second.vbo.destroy();
    }
    id2mesh_.clear();

    entity_shader_.reset();
    entity_uniforms_ = EntityShaderUniforms();

    preview_shader_.reset();
    preview_uniforms_ = PreviewShaderUniforms();
    preview_tex_.reset();
    preview_vao_.destroy();
    preview_vbo_.destroy();

    render_frame_shader_.reset();
    render_frame_uniforms_ = RenderFrameShaderUniforms();
    render_frame_vao_.destroy();
    render_frame_vbo_.destroy();

    doneCurrent();
}

AGZ_EDITOR_END
