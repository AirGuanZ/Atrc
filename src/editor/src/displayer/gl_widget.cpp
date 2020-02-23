#include <QMouseEvent>
#include <QTimer>
#include <QVBoxLayout>

#include <im3d_math.h>

#include <agz/editor/displayer/gl_widget.h>
#include <agz/editor/ui/transform3d_widget.h>
#include <agz/editor/utility/direct_transform3d.h>
#include <agz/utility/system.h>

#ifdef AGZ_DEBUG
#include <QOpenGLDebugLogger>
#include <QOpenGLDebugMessage>
#endif

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
        float light_factor = 0.1 + 0.75 * max(0, dot(normalize(w_normal), -light_dir));
        light_factor = 0.4 * light_factor + 0.6 * light_factor * light_factor;
        light_factor = pow(light_factor, 1 / 1.4);
        frag_color = vec4(light_factor * color, 1);
    }
)___";

    const char *BACKGROUND_VERTEX_SHADER_SOURCE = R"___(
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

    const char *BACKGROUND_FRAGMENT_SHADER_SOURCE = R"___(
    #version 330 core

    uniform sampler2D tex;

    in vec2 uv;

    out vec4 frag_color;

    void main()
    {
        vec3 color = texture2D(tex, uv).rgb;
        frag_color = vec4(pow(color, vec3(1 / 2.2)), 1);
    }
)___";

} // namespace anonymous

DisplayerGLWidget::DisplayerGLWidget()
{
    QHBoxLayout *layout      = new QHBoxLayout(this);
    QPushButton *switch_mode = new QPushButton("Switch Mode", this);
    gizmo_selector_          = new GizmoSelector;

    switch_mode->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    layout->addWidget(switch_mode);
    layout->addWidget(gizmo_selector_);
    layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    connect(switch_mode, &QPushButton::clicked, [=]
    {
        in_realtime_mode_ = !in_realtime_mode_;

        emit switch_render_mode();
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
}

DisplayerGLWidget::~DisplayerGLWidget()
{
    destroy();
}

DisplayerGLWidget::MeshID DisplayerGLWidget::generate_mesh_id()
{
    return next_gen_mesh_id_++;
}

void DisplayerGLWidget::add_mesh(MeshID id, const Vertex *vertices, int vertex_count)
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

void DisplayerGLWidget::remove_mesh(MeshID id)
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

void DisplayerGLWidget::set_transform(MeshID id, const DirectTransform &world)
{
    const auto it = id2mesh_.find(id);
    assert(it != id2mesh_.end());
    it->second.world = world;
    update();
}

void DisplayerGLWidget::set_camera(
    const QMatrix4x4 &proj_view, const Vec3 &cam_pos, const Vec3 &cam_dir,
    const Vec3 &cam_up, real cam_fov_deg)
{
    cam_pos_     = cam_pos;
    cam_dir_     = cam_dir.normalize();
    cam_up_      = cam_up;
    cam_fov_deg_ = cam_fov_deg;
    proj_view_   = proj_view;

    if(im3d_)
    {
        im3d_->set_camera(
            { cam_pos_.x, cam_pos_.y, cam_pos_.z },
            { cam_dir_.x, cam_dir_.y, cam_dir_.z },
            { cam_up_.x, cam_up_.y, cam_up_.z },
            cam_fov_deg_);
    }

    update();
}

void DisplayerGLWidget::set_selected_mesh(MeshID id)
{
    selected_mesh_id_ = id;

    update();
}

void DisplayerGLWidget::set_background_image(const Image2D<Spectrum> &img)
{
    makeCurrent();

    if(img.width() > 0 && img.height() > 0)
    {
        background_tex_ = std::make_unique<QOpenGLTexture>(QOpenGLTexture::Target2D);
        background_tex_->create();
        background_tex_->setSize(img.width(), img.height());
        background_tex_->setFormat(QOpenGLTexture::RGB16F);
        background_tex_->setMipLevels(1);
        background_tex_->allocateStorage(QOpenGLTexture::RGB, QOpenGLTexture::Float16);
        background_tex_->setData(
            QOpenGLTexture::RGB, QOpenGLTexture::Float32, img.raw_data());
        background_tex_->setMinMagFilters(
            QOpenGLTexture::Nearest, QOpenGLTexture::Nearest);
    }
    else
        background_tex_.reset();

    doneCurrent();
    update();
}

bool DisplayerGLWidget::is_in_realtime_mode() const
{
    return in_realtime_mode_;
}

void DisplayerGLWidget::cursor_leave(QEvent *event)
{
    is_left_button_down_ = false;
    update();
}

void DisplayerGLWidget::mouse_move(QMouseEvent *event)
{
    cursor_pos_ = { float(event->x()), float(event->y()) };
    cursor_moved_ = true;
    update();
}

void DisplayerGLWidget::mouse_press(QMouseEvent *event)
{
    if(event->button() != Qt::MouseButton::LeftButton)
        return;

    is_left_button_down_ = true;
    cursor_pos_ = { float(event->x()), float(event->y()) };

    update();
}

void DisplayerGLWidget::mouse_release(QMouseEvent *event)
{
    if(event->button() != Qt::MouseButton::LeftButton)
        return;

    is_left_button_down_ = false;
    cursor_pos_ = { float(event->x()), float(event->y()) };

    update();
}

void DisplayerGLWidget::initializeGL()
{
    // debugger

#ifdef AGZ_DEBUG
    QOpenGLDebugLogger *logger = new QOpenGLDebugLogger(this);
    connect(logger, &QOpenGLDebugLogger::messageLogged, [=](const QOpenGLDebugMessage &msg)
    {
        printf("%s\n", msg.message().toStdString().c_str());
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
    entity_uniforms_.world_loc_     = entity_shader_->uniformLocation("world");
    entity_uniforms_.mvp_loc_       = entity_shader_->uniformLocation("proj_view_world");
    entity_uniforms_.color_loc_     = entity_shader_->uniformLocation("color");
    entity_uniforms_.light_dir_loc_ = entity_shader_->uniformLocation("light_dir");
    entity_shader_->release();

    // background shader

    background_shader_ = std::make_unique<QOpenGLShaderProgram>();
    background_shader_->addShaderFromSourceCode(QOpenGLShader::Vertex,   BACKGROUND_VERTEX_SHADER_SOURCE);
    background_shader_->addShaderFromSourceCode(QOpenGLShader::Fragment, BACKGROUND_FRAGMENT_SHADER_SOURCE);
    background_shader_->link();

    background_shader_->bindAttributeLocation("position", 0);
    background_shader_->bindAttributeLocation("texcoord", 1);

    background_shader_->bind();
    background_uniforms_.background_tex_loc_ = background_shader_->uniformLocation("tex");
    background_shader_->release();

    // background buffer

    background_vao_.create();
    background_vao_.bind();

    background_vbo_.create();
    background_vbo_.bind();

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
    background_vbo_.allocate(background_vertices, sizeof(background_vertices));

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

    background_vbo_.release();
    background_vao_.release();

    // im3d

    im3d_ = std::make_unique<Im3d::Im3dInst>(width(), height());
    im3d_->set_camera(
        { cam_pos_.x, cam_pos_.y, cam_pos_.z },
        { cam_dir_.x, cam_dir_.y, cam_dir_.z },
        { cam_up_.x, cam_up_.y, cam_up_.z },
        cam_fov_deg_);

    im3d_->set_framebuffer(width(), height());
    im3d_->update({ 0, 0 }, false);
}

void DisplayerGLWidget::paintGL()
{
    im3d_->set_framebuffer(width(), height());
    im3d_->update({ cursor_pos_.x, cursor_pos_.y }, is_left_button_down_);
    im3d_->new_frame();

    if(!in_realtime_mode_ && background_tex_)
        render_background_image();

    if(in_realtime_mode_)
        render_entities();

    render_gizmo();

    makeCurrent();

    glDisable(GL_DEPTH_TEST);
    im3d_->draw(QOpenGLContext::currentContext()->functions());
}

void DisplayerGLWidget::render_background_image()
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    background_shader_->bind();
    background_vao_.bind();
    background_tex_->bind(0);

    background_shader_->setUniformValue(
        background_uniforms_.background_tex_loc_, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    background_tex_->release(0);
    background_vao_.release();
    background_shader_->release();
}

void DisplayerGLWidget::render_entities()
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const real cam_hori_rad = tracer::local_angle::phi(cam_dir_);
    const real cam_vert_rad = tracer::local_angle::theta(cam_dir_);

    const QVector3D light_dir = QVector3D(
        std::sin(cam_vert_rad + 0.3f) * std::cos(cam_hori_rad - 0.2f),
        std::sin(cam_vert_rad + 0.3f) * std::sin(cam_hori_rad - 0.2f),
        std::cos(cam_vert_rad + 0.3f)).normalized();

    entity_shader_->bind();

    for(auto &p : id2mesh_)
    {
        Mesh &mesh = p.second;
        mesh.vao.bind();

        const Mat4 world_t = mesh.world.compose().transpose();

        const QMatrix4x4 world(&world_t.data[0][0]);
        const QMatrix4x4 mvp = proj_view_ * world;

        const QVector3D color = p.first == selected_mesh_id_ ?
                                QVector3D(0, 1, 1) :
                                QVector3D(0.7f, 0.7f, 0.7f);

        entity_shader_->setUniformValue(entity_uniforms_.world_loc_, world);
        entity_shader_->setUniformValue(entity_uniforms_.mvp_loc_, mvp);
        entity_shader_->setUniformValue(entity_uniforms_.color_loc_, color);
        entity_shader_->setUniformValue(entity_uniforms_.light_dir_loc_, light_dir);

        glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_count);

        mesh.vao.release();
    }

    entity_shader_->release();
}

void DisplayerGLWidget::render_gizmo()
{
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
            if     (mesh.world.scale != scale[0]) mesh.world.scale = scale[0];
            else if(mesh.world.scale != scale[1]) mesh.world.scale = scale[1];
            else if(mesh.world.scale != scale[2]) mesh.world.scale = scale[2];
        }
    }

    is_edited &= cursor_moved_;
    cursor_moved_ = false;

    Im3d::PopMatrix();

    if(is_edited)
        emit edit_gizmo(mesh.world);
}

void DisplayerGLWidget::destroy()
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
    
    background_shader_.reset();
    background_uniforms_ = BackgroundShaderUniforms();
    background_tex_.reset();
    background_vao_.destroy();
    background_vbo_.destroy();

    doneCurrent();
}

AGZ_EDITOR_END
