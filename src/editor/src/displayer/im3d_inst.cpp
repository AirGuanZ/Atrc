#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <im3d_math.h>

#include <agz/editor/displayer/im3d_inst.h>

namespace
{
    const char *IM3D_SHADER_SOURCE = R"___(
#if !defined(POINTS) && !defined(LINES) && !defined(TRIANGLES)
	#error No primitive type defined
#endif
#if !defined(VERTEX_SHADER) && !defined(GEOMETRY_SHADER) && !defined(FRAGMENT_SHADER)
	#error No shader stage defined
#endif

#define VertexData \
	_VertexData { \
		noperspective float m_edgeDistance; \
		noperspective float m_size; \
		smooth vec4 m_color; \
	}

#define kAntialiasing 2.0

#ifdef VERTEX_SHADER
	uniform mat4 uViewProjMatrix;
	
	layout(location=0) in vec4 aPositionSize;
	layout(location=1) in vec4 aColor;
	
	out VertexData vData;
	
	void main() 
	{
		vData.m_color = aColor.abgr; // swizzle to correct endianness
		#if !defined(TRIANGLES)
			vData.m_color.a *= smoothstep(0.0, 1.0, aPositionSize.w / kAntialiasing);
		#endif
		vData.m_size = max(aPositionSize.w, kAntialiasing);
		gl_Position = uViewProjMatrix * vec4(aPositionSize.xyz, 1.0);
		#if defined(POINTS)
			gl_PointSize = vData.m_size;
		#endif
	}
#endif

#ifdef GEOMETRY_SHADER
 // expand line -> triangle strip
	layout(lines) in;
	layout(triangle_strip, max_vertices = 4) out;
	
	uniform vec2 uViewport;
	
	in  VertexData vData[];
	out VertexData vDataOut;
	
	void main() 
	{
		vec2 pos0 = gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w;
		vec2 pos1 = gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w;
		
		vec2 dir = pos0 - pos1;
		dir = normalize(vec2(dir.x, dir.y * uViewport.y / uViewport.x)); // correct for aspect ratio
		vec2 tng0 = vec2(-dir.y, dir.x);
		vec2 tng1 = tng0 * vData[1].m_size / uViewport;
		tng0 = tng0 * vData[0].m_size / uViewport;
		
	 // line start
		gl_Position = vec4((pos0 - tng0) * gl_in[0].gl_Position.w, gl_in[0].gl_Position.zw); 
		vDataOut.m_edgeDistance = -vData[0].m_size;
		vDataOut.m_size = vData[0].m_size;
		vDataOut.m_color = vData[0].m_color;
		EmitVertex();
		
		gl_Position = vec4((pos0 + tng0) * gl_in[0].gl_Position.w, gl_in[0].gl_Position.zw);
		vDataOut.m_color = vData[0].m_color;
		vDataOut.m_edgeDistance = vData[0].m_size;
		vDataOut.m_size = vData[0].m_size;
		EmitVertex();
		
	 // line end
		gl_Position = vec4((pos1 - tng1) * gl_in[1].gl_Position.w, gl_in[1].gl_Position.zw);
		vDataOut.m_edgeDistance = -vData[1].m_size;
		vDataOut.m_size = vData[1].m_size;
		vDataOut.m_color = vData[1].m_color;
		EmitVertex();
		
		gl_Position = vec4((pos1 + tng1) * gl_in[1].gl_Position.w, gl_in[1].gl_Position.zw);
		vDataOut.m_color = vData[1].m_color;
		vDataOut.m_size = vData[1].m_size;
		vDataOut.m_edgeDistance = vData[1].m_size;
		EmitVertex();
	}
#endif

#ifdef FRAGMENT_SHADER
	in VertexData vData;
	
	layout(location=0) out vec4 fResult;
	
	void main() 
	{
		fResult = vData.m_color;
		
		#if   defined(LINES)
			float d = abs(vData.m_edgeDistance) / vData.m_size;
			d = smoothstep(1.0, 1.0 - (kAntialiasing / vData.m_size), d);
			fResult.a *= d;
			
		#elif defined(POINTS)
			float d = length(gl_PointCoord.xy - vec2(0.5));
			d = smoothstep(0.5, 0.5 - (kAntialiasing / vData.m_size), d);
			fResult.a *= d;
			
		#endif		
	}
#endif
)___";
}

namespace Im3d
{

Im3dInst::Im3dInst(int width, int height)
{
    fb_width_  = width;
    fb_height_ = height;

    cam_pos_ = { 0, 0, 0 };
    cam_dir_ = { 0, 1, 0 };
    cam_fov_deg_ = 60.0f;

    cam_view_ = Mat4(1);
    cam_proj_ = Mat4(1);
    cam_proj_view_ = Mat4(1);

    init_im3d();
}

Im3dInst::~Im3dInst()
{
    destroy_im3d();
}

void Im3dInst::update(const Vec2 &cursor_pos, bool left_button_down)
{
    AppData &ad = GetAppData();

    ad.m_deltaTime     = 1;
    ad.m_viewportSize  = { static_cast<float>(fb_width_), static_cast<float>(fb_height_) };
    ad.m_viewOrigin    = cam_pos_;
    ad.m_viewDirection = cam_dir_;
    ad.m_worldUp       = { 0, 0, 1 };
    ad.m_projOrtho     = false;

    const float fov_rad = agz::math::deg2rad(cam_fov_deg_);
    ad.m_projScaleY = 2 * std::tan(0.5f * fov_rad);

    curr_cursor_pos_ = cursor_pos;
    curr_cursor_pos_ = 2 * curr_cursor_pos_ / ad.m_viewportSize - Vec2(1);
    curr_cursor_pos_.y = -curr_cursor_pos_.y;

    Vec3 ray_dir;
    ray_dir.x = curr_cursor_pos_.x / cam_proj_(0, 0);
    ray_dir.y = curr_cursor_pos_.y / cam_proj_(1, 1);
    ray_dir.z = -1;
    ray_dir = cam_world_ * Vec4(Normalize(ray_dir), 0);

    ad.m_cursorRayOrigin    = cam_pos_;
    ad.m_cursorRayDirection = ray_dir;

    ad.setCullFrustum(cam_proj_view_, true);

    ad.m_keyDown[Mouse_Left] = left_button_down;
    ad.m_keyDown[Key_L]      = false;
    ad.m_keyDown[Key_T]      = false;
    ad.m_keyDown[Key_R]      = false;
    ad.m_keyDown[Key_S]      = false;

    ad.m_snapTranslation = 0;
    ad.m_snapRotation    = 0;
    ad.m_snapScale       = 0;
}

void Im3dInst::new_frame()
{
    NewFrame();
}

void Im3dInst::draw(QOpenGLFunctions *gl)
{
    EndFrame();

    auto &ad = GetAppData();

    gl->glEnable(GL_BLEND);
    gl->glBlendEquation(GL_FUNC_ADD);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl->glEnable(GL_PROGRAM_POINT_SIZE);

    for(U32 i = 0, n = GetDrawListCount(); i < n; ++i)
    {
        auto &draw_list = GetDrawLists()[i];

        GLenum prim;
        QOpenGLShaderProgram *shader;

        switch(draw_list.m_primType)
        {
        case DrawPrimitive_Points:
            prim = GL_POINTS;
            shader = &shader_points_;
            gl->glDisable(GL_CULL_FACE);
            break;
        case DrawPrimitive_Lines:
            prim = GL_LINES;
            shader = &shader_lines_;
            gl->glDisable(GL_CULL_FACE);
            break;
        default: // DrawPrimitive_Triangles:
            prim = GL_TRIANGLES;
            shader = &shader_triangles_;
            break;
        }

        QOpenGLVertexArrayObject vao;
        vao.create();
        vao.bind();

        QOpenGLBuffer vbo;
        vbo.create();
        vbo.bind();
        vbo.allocate(draw_list.m_vertexData, sizeof(VertexData) * draw_list.m_vertexCount);

        gl->glEnableVertexAttribArray(0);
        gl->glEnableVertexAttribArray(1);
        gl->glVertexAttribPointer(
            0, 4, GL_FLOAT, GL_FALSE,
            sizeof(VertexData), (GLvoid *)offsetof(VertexData, m_positionSize));
        gl->glVertexAttribPointer(
            1, 4, GL_UNSIGNED_BYTE, GL_TRUE,
            sizeof(VertexData), (GLvoid *)offsetof(VertexData, m_color));

        shader->bind();

        gl->glUniform2f(
            shader->uniformLocation("uViewport"),
            ad.m_viewportSize.x, ad.m_viewportSize.y);
        gl->glUniformMatrix4fv(
            shader->uniformLocation("uViewProjMatrix"),
            1, false, (const GLfloat *)cam_proj_view_);

        gl->glDrawArrays(prim, 0, (GLsizei)draw_list.m_vertexCount);

        shader->release();
        vbo.release();
        vao.release();
    }

    gl->glDisable(GL_BLEND);
    gl->glDisable(GL_PROGRAM_POINT_SIZE);
}

void Im3dInst::set_framebuffer(int width, int height)
{
    fb_width_  = width;
    fb_height_ = height;
}

void Im3dInst::set_camera(
    const Vec3 &pos, const Vec3 &dir, const Vec3 &up, float fov_deg)
{
    cam_pos_     = pos;
    cam_dir_     = dir;
    cam_up_      = up;
    cam_fov_deg_ = fov_deg;

    // view matrix

    QMatrix4x4 view;
    view.lookAt(
        { pos.x, pos.y, pos.z },
        { pos.x + dir.x, pos.y + dir.y, pos.z + dir.z },
        { up.x, up.y, up.z });
    std::memcpy(&cam_view_, view.constData(), sizeof(Mat4));

    cam_world_ = Inverse(cam_view_);

    // proj matrix

    const float aspect = static_cast<float>(fb_width_) / fb_height_;

    QMatrix4x4 proj;
    proj.perspective(
        cam_fov_deg_, aspect, 0.1f, 1000);
    memcpy(&cam_proj_, proj.constData(), sizeof(Mat4));

    cam_proj_view_ = cam_proj_ * cam_view_;
}

void Im3dInst::init_im3d()
{
    {
        const QString vs_src = QString(
            "#version 330 core\n#define VERTEX_SHADER\n#define POINTS\n") + IM3D_SHADER_SOURCE;
        const QString fs_src = QString(
            "#version 330 core\n#define FRAGMENT_SHADER\n#define POINTS\n") + IM3D_SHADER_SOURCE;

        shader_points_.addShaderFromSourceCode(QOpenGLShader::Vertex,   vs_src);
        shader_points_.addShaderFromSourceCode(QOpenGLShader::Fragment, fs_src);
        shader_points_.link();
    }

    {
        const QString vs_src = QString(
            "#version 330 core\n#define VERTEX_SHADER\n#define LINES\n") + IM3D_SHADER_SOURCE;
        const QString gs_src = QString(
            "#version 330 core\n#define GEOMETRY_SHADER\n#define LINES\n") + IM3D_SHADER_SOURCE;
        const QString fs_src = QString(
            "#version 330 core\n#define FRAGMENT_SHADER\n#define LINES\n") + IM3D_SHADER_SOURCE;

        shader_lines_.addShaderFromSourceCode(QOpenGLShader::Vertex,   vs_src);
        shader_lines_.addShaderFromSourceCode(QOpenGLShader::Geometry, gs_src);
        shader_lines_.addShaderFromSourceCode(QOpenGLShader::Fragment, fs_src);
        shader_lines_.link();
    }

    {
        const QString vs_src = QString(
            "#version 330 core\n#define VERTEX_SHADER\n#define TRIANGLES\n") + IM3D_SHADER_SOURCE;
        const QString fs_src = QString(
            "#version 330 core\n#define FRAGMENT_SHADER\n#define TRIANGLES\n") + IM3D_SHADER_SOURCE;

        shader_triangles_.addShaderFromSourceCode(QOpenGLShader::Vertex,   vs_src);
        shader_triangles_.addShaderFromSourceCode(QOpenGLShader::Fragment, fs_src);
        shader_triangles_.link();
    }

    Im3d::GetContext().m_gizmoHeightPixels = 128;
}

void Im3dInst::destroy_im3d()
{
    shader_points_.destroyed();
    shader_lines_.destroyed();
    shader_triangles_.destroyed();
}

} // namespace Im3d
