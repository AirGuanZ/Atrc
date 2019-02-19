#include <Atrc/Editor/ResourceManagement/EntityCreator.h>
#include <Atrc/Editor/EntityController.h>
#include <Lib/imgui/imgui/ImGuizmo.h>

namespace
{
    const char VS_SRC[] = R"____(
    #version 450 core
    uniform mat4 WVP;
    uniform mat4 WORLD;
    uniform vec3 EYE_POS;
    in vec3 lPos;
    in vec3 lNor;
    out vec3 wNor;
    void main(void)
    {
        gl_Position = WVP * vec4(lPos, 1);
        wNor = (WORLD * vec4(lNor, 0)).xyz;
        vec4 wPos = WORLD * vec4(lPos, 1);
        if(dot(wNor, EYE_POS - wPos.xyz / wPos.w) < 0)
            wNor = -wNor;
    }
    )____";

    const char FS_SRC[] = R"____(
    #version 450 core
    uniform vec3 COLOR;
    in vec3 wNor;
    out vec4 fragColor;
    void main(void)
    {
        float lightFactor = 0.9 * max(0, dot(normalize(vec3(1, 1, 1)), normalize(wNor))) + 0.1;
        fragColor = vec4(min(lightFactor, 1) * COLOR, 1);
    }
    )____";

    GL::Program prog;

    GL::UniformVariable<Mat4f> uniformWVP;
    GL::UniformVariable<Mat4f> uniformWORLD;
    GL::UniformVariable<Vec3f> uniformEYE_POS;
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
        uniformEYE_POS = prog.GetUniformVariable<Vec3f>("EYE_POS");
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
    protected:

        Vec3f renderColor_ = Vec3f(0.5f);
        GeometrySlot geometry_;
        EntityController controller_;

        void DisplayRenderProperty()
        {
            ImGui::ColorEdit3("render color", &renderColor_[0]);
        }

        void DisplayController(const Mat4f &proj, const Mat4f &view, bool renderController)
        {
            controller_.Display(proj, view, renderController);
        }

    public:

        explicit TransformedGeometricEntityBase(ResourceManager &rscMgr, std::string typeName, std::string name)
            : EntityInstance(std::move(typeName), std::move(name))
        {
            CheckRendererInitialization();
            geometry_.SetInstance(rscMgr.Create<GeometryInstance>("TriangleBVH", ""));
        }

        void Render(const Mat4f &projViewMat, const Vec3f &eyePos) override
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

            Mat4f world = GetFinalMatrix(controller_.GetTranslate(), controller_.GetRotate(), controller_.GetScale());
            Mat4f WVP = projViewMat * world;

            uniformWVP.BindValue(WVP);
            uniformWORLD.BindValue(world);
            uniformEYE_POS.BindValue(eyePos);
            uniformCOLOR.BindValue(renderColor_);

            GL::RenderContext::DrawVertices(GL_TRIANGLES, 0, vtxBuf->GetVertexCount());

            vao.Unbind();
        }

        void DisplayEx(ResourceManager &rscMgr, const Mat4f &proj, const Mat4f &view, bool renderController) override
        {
            geometry_.Display(rscMgr);
            if(auto geo = geometry_.GetInstance())
            {
                geo->DisplayEditing(controller_.GetFinalMatrix(), proj, view,
                    renderController && controller_.GetControllerMode() == EntityControllerAction::Edit);
            }
        }
    };

    class GeometricDiffuseLightInstance : public TransformedGeometricEntityBase
    {
        Vec3f radiance_ = Vec3f(1);

    protected:

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = GeometricDiffuse;");
            ctx.entityController = &controller_;
            ExportSubResource("geometry", rscMgr, ctx, geometry_);
            ctx.entityController = nullptr;
            ctx.AddLine("radiance = ", AGZ::To<char>(radiance_), ";");
        }

    public:

        using TransformedGeometricEntityBase::TransformedGeometricEntityBase;

        void DisplayEx(ResourceManager &rscMgr, const Mat4f &proj, const Mat4f &view, bool renderController) override
        {
            DisplayRenderProperty();
            if(ImGui::TreeNode("geometry"))
            {
                TransformedGeometricEntityBase::DisplayEx(rscMgr, proj, view, renderController);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("radiance"))
            {
                ImGui::ColorEdit3("radiance", &radiance_[0], ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
                ImGui::TreePop();
            }
            DisplayController(proj, view, renderController);
        }
    };

    class GeometricEntityInstance : public TransformedGeometricEntityBase
    {
        MaterialSlot material_;

    protected:

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = Geometric;");
            ctx.entityController = &controller_;
            ExportSubResource("geometry", rscMgr, ctx, geometry_);
            ctx.entityController = nullptr;
            ExportSubResource("material", rscMgr, ctx, material_);
        }

    public:

        using TransformedGeometricEntityBase::TransformedGeometricEntityBase;

        void DisplayEx(ResourceManager &rscMgr, const Mat4f &proj, const Mat4f &view, bool renderController) override
        {
            DisplayRenderProperty();
            if(ImGui::TreeNode("geometry"))
            {
                TransformedGeometricEntityBase::DisplayEx(rscMgr, proj, view, renderController);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("material"))
            {
                material_.Display(rscMgr);
                ImGui::TreePop();
            }
            DisplayController(proj, view, renderController);
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

std::shared_ptr<EntityInstance> GeometricDiffuseLightCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<GeometricDiffuseLightInstance>(rscMgr, GetName(), std::move(name));
}

std::shared_ptr<EntityInstance> GeometricEntityCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<GeometricEntityInstance>(rscMgr, GetName(), std::move(name));
}
