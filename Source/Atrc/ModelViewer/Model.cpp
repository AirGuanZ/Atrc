#include "GL.h"
#include "Model.h"

namespace
{
    const char VS_SRC[] = R"____(
    #version 450 core
    uniform mat4 WVP;
    uniform mat4 WORLD;
    in vec3 lPos;
    in vec3 lNor;
    out vec3 wNor;
    void main(void)
    {
        gl_Position = WVP * vec4(lPos, 1);
        wNor = (WORLD * vec4(lNor, 0)).xyz;
    }
    )____";

    const char FS_SRC[] = R"____(
    #version 450 core
    uniform vec3 COLOR;
    in vec3 wNor;
    out vec4 fragColor;
    void main(void)
    {
        float lightFactor = max(0, dot(normalize(vec3(1, 1, 1)), normalize(wNor)));
        fragColor = vec4(lightFactor * COLOR, 1);
    }
    )____";

    GL::Program prog;

    GL::UniformVariable<Mat4f> uniformWVP;
    GL::UniformVariable<Mat4f> uniformWORLD;
    GL::UniformVariable<Vec3f> uniformCOLOR;

    GL::AttribVariable<Vec3f> attribLPos;
    GL::AttribVariable<Vec3f> attribLNor;

    void CheckRendererInitialization()
    {
        if(prog.GetHandle())
            return;

        prog = GL::ProgramBuilder::BuildOnce(
            GL::VertexShader::FromMemory(VS_SRC), GL::FragmentShader::FromMemory(FS_SRC));

        uniformWVP   = prog.GetUniformVariable<Mat4f>("WVP");
        uniformWORLD = prog.GetUniformVariable<Mat4f>("WORLD");
        uniformCOLOR = prog.GetUniformVariable<Vec3f>("COLOR");

        attribLPos = prog.GetAttribVariable<Vec3f>("lPos");
        attribLNor = prog.GetAttribVariable<Vec3f>("lNor");
    }
} // namespace null

void Model::BeginRendering()
{
    prog.Bind();
}

void Model::EndRendering()
{
    prog.Unbind();
}

Model::Model(std::string name) noexcept
    : name_(std::move(name)), displayColorWith255_(false)
{
    
}

void Model::Initialize(std::shared_ptr<GL::VertexBuffer<Vertex>> vtxBuf, const Vec3f &renderColor)
{
    AGZ_ASSERT(vtxData && vtxCount);
    CheckRendererInitialization();

    vtxBuf_ = std::move(vtxBuf);

    vao_.InitializeHandle();
    vao_.EnableAttrib(attribLPos);
    vao_.EnableAttrib(attribLNor);
    vao_.BindVertexBufferToAttrib(attribLPos, *vtxBuf_, &Vertex::pos, 0);
    vao_.BindVertexBufferToAttrib(attribLNor, *vtxBuf_, &Vertex::nor, 1);

    AGZ_ASSERT(!displayColorWith255_);
    renderColor_ = renderColor;
    displayColor_ = renderColor;
}

void Model::Render(const Camera &camera) const
{
    vao_.Bind();

    Mat4f WVP = camera.GetProjMatrix() * camera.GetViewMatrix() * transSeq_.GetFinalTransformMatrix();
    uniformWVP.BindValue(WVP);
    uniformWORLD.BindValue(transSeq_.GetFinalTransformMatrix());
    uniformCOLOR.BindValue(renderColor_);

    GL::RenderContext::DrawVertices(GL_TRIANGLES, 0, vtxBuf_->GetVertexCount());

    vao_.Unbind();
}

void Model::DisplayProperty()
{
    if(ImGui::InputFloat3("color", &displayColor_[0]) || ImGui::Checkbox("0..255", &displayColorWith255_))
    {
        if(displayColorWith255_)
            renderColor_ = displayColor_ / 255;
        else
            renderColor_ = displayColor_;
    }
}

void Model::DisplayTransformSeq()
{
    transSeq_.Display();
}
