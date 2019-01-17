#include "ModelRenderer.h"

namespace
{
    const char *VS = R"____(
    #version 450 core
    uniform mat4 WVP;
    uniform mat4 WORLD;
    in vec3 iPos;
    in vec3 iNor;
    in vec2 iTex;
    out vec3 wNor;
    out vec2 mTex;
    void main(void)
    {
        gl_Position = WVP * vec4(iPos, 1);
        wNor = (WORLD * vec4(iNor, 0)).xyz;
        mTex = iTex;
    }
    )____";

    const char *FS = R"____(
    #version 450 core
    uniform sampler2D tex;
    in vec3 wNor;
    in vec2 mTex;
    out vec4 fragColor;
    void main(void)
    {
        fragColor = max(0, dot(normalize(wNor), normalize(vec3(-2, 1, 1)))) * max(vec4(1), texture(tex, mTex));
    }
    )____";
} // namespace null

void ModelRenderer::Initialize()
{
    vtxBuf_.InitializeHandle();
    elemBuf_.InitializeHandle();

    program_ = GL::ProgramBuilder::BuildOnce(
        GL::VertexShader::FromMemory(VS), GL::FragmentShader::FromMemory(FS));
    uniforms_.SetValue(program_.GetUniformVariable<GL::Texture2DUnit>("tex"), GL::Texture2DUnit{ 0 });

    auto iPos = program_.GetAttribVariable<Vec3f>("iPos");
    auto iNor = program_.GetAttribVariable<Vec3f>("iNor");
    auto iTex = program_.GetAttribVariable<Vec2f>("iTex");
    vao_.InitializeHandle();
    vao_.EnableAttrib(iPos);
    vao_.EnableAttrib(iNor);
    vao_.EnableAttrib(iTex);
    vao_.BindVertexBufferToAttrib(iPos, vtxBuf_, &Vertex::pos, 0);
    vao_.BindVertexBufferToAttrib(iNor, vtxBuf_, &Vertex::nor, 1);
    vao_.BindVertexBufferToAttrib(iTex, vtxBuf_, &Vertex::tex, 2);
    vao_.BindElementBuffer(elemBuf_);
}

void ModelRenderer::SetModelData(const Vertex *vtx, uint32_t vtxCount, const uint32_t *elem, uint32_t elemCount)
{
    vtxBuf_.ReinitializeData(vtx, vtxCount, GL_STATIC_DRAW);
    elemBuf_.ReinitializeData(elem, elemCount, GL_STATIC_DRAW);
}

void ModelRenderer::SetWorld(const Mat4f &world)
{
    world_ = world;
    WVP_ = proj_ * view_ * world_;
    uniforms_.SetValue(program_.GetUniformVariable<Mat4f>("WVP"), WVP_);
    uniforms_.SetValue(program_.GetUniformVariable<Mat4f>("WORLD"), world_);
}

void ModelRenderer::SetView(const Mat4f &view)
{
    view_ = view;
    WVP_ = proj_ * view_ * world_;
    uniforms_.SetValue(program_.GetUniformVariable<Mat4f>("WVP"), WVP_);
    uniforms_.SetValue(program_.GetUniformVariable<Mat4f>("WORLD"), world_);
}

void ModelRenderer::SetProj(const Mat4f &proj)
{
    proj_ = proj;
    WVP_ = proj_ * view_ * world_;
    uniforms_.SetValue(program_.GetUniformVariable<Mat4f>("WVP"), WVP_);
    uniforms_.SetValue(program_.GetUniformVariable<Mat4f>("WORLD"), world_);
}

void ModelRenderer::Render() const
{
    program_.Bind();
    vao_.Bind();
    uniforms_.Bind();
    tex_->Bind(0);

    GL::RenderContext::DrawElements(GL_TRIANGLES, 0, elemBuf_.GetElemCount(), elemBuf_.GetElemType());

    tex_->Unbind(0);
    vao_.Unbind();
    program_.Unbind();
}
