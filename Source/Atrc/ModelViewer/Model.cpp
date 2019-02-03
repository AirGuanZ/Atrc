#include <Lib/imgui/imgui/ImGuizmo.h>

#include "GL.h"
#include "Model.h"
#include "TransformController.h"

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
        float lightFactor = max(0, dot(normalize(vec3(1, 1, 1)), normalize(wNor))) + 0.1;
        fragColor = vec4(min(lightFactor, 1) * COLOR, 1);
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
    : name_(std::move(name))
{
    
}

void Model::Initialize(std::shared_ptr<const GL::VertexBuffer<Vertex>> vtxBuf, const Vec3f &renderColor)
{
    CheckRendererInitialization();

    vtxBuf_ = std::move(vtxBuf);

    vao_.InitializeHandle();
    vao_.EnableAttrib(attribLPos);
    vao_.EnableAttrib(attribLNor);
    vao_.BindVertexBufferToAttrib(attribLPos, *vtxBuf_, &Vertex::pos, 0);
    vao_.BindVertexBufferToAttrib(attribLNor, *vtxBuf_, &Vertex::nor, 1);

    renderColor_ = renderColor;
}

namespace
{
    Mat4f GetFinalMatrix(const Vec3f &translate, const Vec3f &rotate, float scale)
    {
        float scaleVec[3] = { scale, scale, scale };
        Mat4f ret;
        ImGuizmo::RecomposeMatrixFromComponents(&translate[0], &rotate[0], scaleVec, &ret.m[0][0]);
        return ret;
    }
}

void Model::Render(const Camera &camera) const
{
    vao_.Bind();

    Mat4f world = GetFinalMatrix(transform_.translate, transform_.rotate, transform_.scale);
    Mat4f WVP = camera.GetProjMatrix() * camera.GetViewMatrix() * world;

    uniformWVP.BindValue(WVP);
    uniformWORLD.BindValue(world);
    uniformCOLOR.BindValue(renderColor_);

    GL::RenderContext::DrawVertices(GL_TRIANGLES, 0, vtxBuf_->GetVertexCount());

    vao_.Unbind();
}

void Model::DisplayProperty()
{
    ImGui::ColorEdit3("color", &renderColor_[0], ImGuiColorEditFlags_HDR);
}

void Model::DisplayTransform(const Camera &camera)
{
    transformController_.Render(camera, &transform_.translate, &transform_.rotate, &transform_.scale);
    transformController_.Display();

    ImGui::InputFloat3("translate##input_translate", &transform_.translate[0]);
    ImGui::InputFloat3("rotate##input_rotate",       &transform_.rotate[0]);
    ImGui::InputFloat("scale##input_scale",          &transform_.scale);
}
