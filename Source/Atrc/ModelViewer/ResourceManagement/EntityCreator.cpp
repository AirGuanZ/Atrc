#include <Lib/imgui/imgui/ImGuizmo.h>

#include <Atrc/ModelViewer/ResourceManagement/EntityCreator.h>
#include <Atrc/ModelViewer/MaterialMapping.h>
#include <Atrc/ModelViewer/TransformController.h>

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

    GL::VertexArray vao;

    void CheckRendererInitialization()
    {
        if(prog.GetHandle())
            return;

        prog = GL::ProgramBuilder::BuildOnce(
            GL::VertexShader::FromMemory(VS_SRC), GL::FragmentShader::FromMemory(FS_SRC));

        uniformWVP = prog.GetUniformVariable<Mat4f>("WVP");
        uniformWORLD = prog.GetUniformVariable<Mat4f>("WORLD");
        uniformCOLOR = prog.GetUniformVariable<Vec3f>("COLOR");

        attribLPos = prog.GetAttribVariable<Vec3f>("lPos");
        attribLNor = prog.GetAttribVariable<Vec3f>("lNor");

        vao.InitializeHandle();
        vao.EnableAttrib(attribLPos);
        vao.EnableAttrib(attribLNor);
    }

    Mat4f GetFinalMatrix(const Vec3f &translate, const Vec3f &rotate, float scale)
    {
        float scaleVec[3] = { scale, scale, scale };
        Mat4f ret;
        ImGuizmo::RecomposeMatrixFromComponents(&translate[0], &rotate[0], scaleVec, &ret.m[0][0]);
        return ret;
    }

    class TransformedGeometricEntityBase : public EntityInstance
    {
        Vec3f renderColor_ = Vec3f(0.5f);
        GeometrySlot geometry_;
        Transform transform_;

    protected:

        void DisplayRenderProperty()
        {
            ImGui::ColorEdit3("render color", &renderColor_[0]);
        }

    public:

        explicit TransformedGeometricEntityBase(std::string name)
            : EntityInstance(std::move(name))
        {
            CheckRendererInitialization();
        }

        void Display(ResourceManager &rscMgr) override
        {
            geometry_.Display(rscMgr);
        }

        void Render(const Camera &camera) override
        {
            std::shared_ptr<const GL::VertexBuffer<GeometryInstance::Vertex>> vtxBuf;
            auto geo = geometry_.GetInstance();
            if(geo)
                vtxBuf = geo->GetVertexBuffer();
            if(!vtxBuf)
                return;

            vao.BindVertexBufferToAttrib(attribLPos, *vtxBuf, &GeometryInstance::Vertex::pos, 0);
            vao.BindVertexBufferToAttrib(attribLNor, *vtxBuf, &GeometryInstance::Vertex::nor, 1);

            vao.Bind();

            Mat4f world = GetFinalMatrix(transform_.GetTranslate(), transform_.GetRotate(), transform_.GetScale());
            Mat4f WVP = camera.GetProjMatrix() * camera.GetViewMatrix() * world;

            uniformWVP.BindValue(WVP);
            uniformWORLD.BindValue(world);
            uniformCOLOR.BindValue(renderColor_);

            GL::RenderContext::DrawVertices(GL_TRIANGLES, 0, vtxBuf->GetVertexCount());

            vao.Unbind();
        }

        void DisplayTransform(const Camera &camera) override
        {
            transform_.Display(camera);
        }
    };

    class GeometricDiffuseLightInstance : public TransformedGeometricEntityBase
    {
        Vec3f radiance_ = Vec3f(1);

    public:

        using TransformedGeometricEntityBase::TransformedGeometricEntityBase;

        void Display(ResourceManager &rscMgr) override
        {
            DisplayRenderProperty();
            if(ImGui::TreeNode("geometry"))
            {
                TransformedGeometricEntityBase::Display(rscMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("radiance"))
            {
                ImGui::ColorEdit3("radiance", &radiance_[0], ImGuiColorEditFlags_HDR);
                ImGui::TreePop();
            }
        }
    };

    class GeometricEntityInstance : public TransformedGeometricEntityBase
    {
        MaterialMappingSelector material_;

    public:

        using TransformedGeometricEntityBase::TransformedGeometricEntityBase;

        void Display(ResourceManager &rscMgr) override
        {
            DisplayRenderProperty();
            if(ImGui::TreeNode("geometry"))
            {
                TransformedGeometricEntityBase::Display(rscMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("material"))
            {
                material_.Display(rscMgr);
                ImGui::TreePop();
            }
        }
    };
}

void BeginEntityRendering()
{
    if(prog.GetHandle())
        prog.Bind();
}

void EndEntityRendering()
{
    if(prog.GetHandle())
        prog.Unbind();
}

void RegisterEntityCreators(ResourceManager &rscMgr)
{
    static const GeometricDiffuseLightCreator iGeometricDiffuseLightCreator;
    static const GeometricEntityCreator iGeometricEntityCreator;
    rscMgr.AddCreator(&iGeometricDiffuseLightCreator);
    rscMgr.AddCreator(&iGeometricEntityCreator);
}

std::shared_ptr<EntityInstance> GeometricDiffuseLightCreator::Create(std::string name) const
{
    return std::make_shared<GeometricDiffuseLightInstance>(std::move(name));
}

std::shared_ptr<EntityInstance> GeometricEntityCreator::Create(std::string name) const
{
    return std::make_shared<GeometricEntityInstance>(std::move(name));
}
