#include <AGZUtils/Utils/Mesh.h>

#include <Atrc/Editor/ResourceManagement/GeometryCreator.h>
#include <Atrc/Editor/FileBrowser.h>
#include <Atrc/Editor/FilenameSlot.h>
#include <Atrc/Editor/Global.h>
#include <Atrc/Mgr/Parser.h>
#include <Lib/imgui/imgui/ImGuizmo.h>

namespace
{
    void ExportTransform(LauncherScriptExportingContext &ctx)
    {
        AGZ_ASSERT(ctx.entityController);
        ctx.AddLine("transform = (");
        ctx.IncIndent();
        ctx.AddLine("Translate",    AGZ::To<char> (ctx.entityController->GetTranslate()), ",");
        ctx.AddLine("RotateZ(Deg(", std::to_string(ctx.entityController->GetRotate().z),  ")),");
        ctx.AddLine("RotateY(Deg(", std::to_string(ctx.entityController->GetRotate().y),  ")),");
        ctx.AddLine("RotateX(Deg(", std::to_string(ctx.entityController->GetRotate().x),  ")),");
        ctx.AddLine("Scale(",       std::to_string(ctx.entityController->GetScale()),     ")");
        ctx.DecIndent();
        ctx.AddLine(");");
    }

    class TriangleInstance : public GeometryInstance
    {
        std::shared_ptr<GL::VertexBuffer<Vertex>> vtxBuf_;
        Vec3f A_, B_, C_;
        Vec3f nor_;
        int editingVtx_;

        void UpdateNormal()
        {
            nor_ = Cross(B_ - A_, C_ - A_);
            if(nor_.Length() < 0.001f)
                nor_ = Vec3f::UNIT_X();
            else
                nor_ = nor_.Normalize();
        }

        void UpdateVertexBuffer()
        {
            if(!vtxBuf_)
                vtxBuf_ = std::make_shared<GL::VertexBuffer<Vertex>>(true);
            Vertex vtxData[] =
            {
                { A_, nor_ },
                { B_, nor_ },
                { C_, nor_ },
            };
            vtxBuf_->ReinitializeData(vtxData, 3, GL_STATIC_DRAW);
        }

    protected:

        void Export(const ResourceManager&, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = Triangle;");
            ctx.AddLine("A = ", AGZ::To<char>(A_), ";");
            ctx.AddLine("B = ", AGZ::To<char>(B_), ";");
            ctx.AddLine("C = ", AGZ::To<char>(C_), ";");
            ctx.AddLine("tA = (0, 0);");
            ctx.AddLine("tB = (0, 0);");
            ctx.AddLine("tC = (0, 0);");
            ExportTransform(ctx);
        }

    public:

        explicit TriangleInstance(std::string typeName, std::string name)
            : GeometryInstance(std::move(typeName), std::move(name)),
              A_(-1, -1, 0), B_(0, 1, 0), C_(1, -1, 0),
              editingVtx_(0)
        {
            UpdateNormal();
            UpdateVertexBuffer();
        }

        std::shared_ptr<const GL::VertexBuffer<Vertex>> GetVertexBuffer() const override
        {
            return vtxBuf_;
        }

        std::vector<std::string> GetSubmeshNames() const override
        {
            return { };
        }

        void DisplayEditing(const Mat4f &world, const Mat4f &proj, const Mat4f &view, bool renderController) override
        {
            bool ABCChanged = false;

            ABCChanged |= ImGui::InputFloat3("A", &A_[0]);
            ABCChanged |= ImGui::InputFloat3("B", &B_[0]);
            ABCChanged |= ImGui::InputFloat3("C", &C_[0]);

            ImGui::RadioButton("A", &editingVtx_, 0); ImGui::SameLine();
            ImGui::RadioButton("B", &editingVtx_, 1); ImGui::SameLine();
            ImGui::RadioButton("C", &editingVtx_, 2);

            if(ABCChanged)
            {
                UpdateNormal();
                UpdateVertexBuffer();
            }

            if(!renderController)
                return;

            Vec3f *editedVtx;
            switch(editingVtx_)
            {
            case 0: editedVtx = &A_; break;
            case 1: editedVtx = &B_; break;
            case 2: editedVtx = &C_; break;
            default: AGZ::Unreachable();
            }

            ImGuiIO &io = ImGui::GetIO();
            ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
            
            Mat4f worldMat = world * Mat4f::Translate(*editedVtx);
            ImGuizmo::Manipulate(&view.m[0][0], &proj.m[0][0], ImGuizmo::TRANSLATE, ImGuizmo::WORLD, &worldMat.m[0][0]);

            //float r[3], s[3];
            worldMat = world.Inverse() * worldMat;
            //ImGuizmo::DecomposeMatrixToComponents(&worldMat.m[0][0], &editedVtx->x, r, s);
            *editedVtx = worldMat.m[3].xyz();
            UpdateNormal();
            UpdateVertexBuffer();
        }

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            A_ = Atrc::Mgr::Parser::TFloat::ParseVec3<float>(params["A"]);
            B_ = Atrc::Mgr::Parser::TFloat::ParseVec3<float>(params["A"]);
            C_ = Atrc::Mgr::Parser::TFloat::ParseVec3<float>(params["A"]);
            UpdateNormal();
            UpdateVertexBuffer();
        }
    };

    class WavefrontOBJInstance : public GeometryInstance
    {
        std::shared_ptr<GL::VertexBuffer<Vertex>> vtxBuf_;
        AGZ::Mesh::GeometryMeshGroup<float> meshGroup_;
        TFilenameSlot<true> filename_;

        void BuildVertexBufferFromMeshGroup()
        {
            std::shared_ptr<GL::VertexBuffer<Vertex>> buf = std::make_shared<GL::VertexBuffer<Vertex>>();
            std::vector<Vertex> vtxData;

            for(auto &it : meshGroup_.submeshes)
            {
                for(auto &v : it.second.vertices)
                {
                    Vertex nv;
                    nv.pos = v.pos;
                    nv.nor = v.nor;
                    vtxData.push_back(nv);
                }
            }

            buf->InitializeHandle();
            buf->ReinitializeData(vtxData.data(), static_cast<uint32_t>(vtxData.size()), GL_STATIC_DRAW);
            vtxBuf_ = std::move(buf);
        }

        void Clear()
        {
            vtxBuf_ = nullptr;
            meshGroup_.submeshes.clear();
        }

        void LoadWavefrontOBJ(std::string filename)
        {
            Clear();
            AGZ::ScopeGuard clearGuard([&]
            {
                Global::ShowErrorMessage("failed to load wavefront obj from " + filename);
                Clear();
                filename_.Clear();
            });

            AGZ::Mesh::WavefrontObj<float> obj;
            if(!obj.LoadFromFile(filename) || obj.name2Obj.empty())
                return;

            meshGroup_ = obj.ToGeometryMeshGroup();
            if(meshGroup_.submeshes.empty())
                return;
            obj.Clear();
            BuildVertexBufferFromMeshGroup();
            clearGuard.Dismiss();
        }

    protected:

        void Export(const ResourceManager&, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = TriangleBVH;");
            ctx.AddLine("filename = \"", filename_.GetExportedFilename(ctx), "\";");
            ExportTransform(ctx);
        }

    public:

        using GeometryInstance::GeometryInstance;

        std::shared_ptr<const GL::VertexBuffer<Vertex>> GetVertexBuffer() const override
        {
            return vtxBuf_;
        }

        std::vector<std::string> GetSubmeshNames() const override
        {
            std::vector<std::string> ret;
            std::transform(begin(meshGroup_.submeshes), end(meshGroup_.submeshes), std::back_inserter(ret),
                [](auto &it) { return it.first; });
            return ret;
        }

        void DisplayEditing(const Mat4f &, const Mat4f&, const Mat4f&, bool) override
        {
            static FileBrowser fileBrowser("browse .obj", false, "");
            if(filename_.Display(fileBrowser))
                LoadWavefrontOBJ(fileBrowser.GetResult());
        }

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            filename_.Import(params["filename"].AsValue(), ctx);
            LoadWavefrontOBJ(filename_.GetFilename());
        }
    };
}

void RegisterGeometryCreators(ResourceManager &rscMgr)
{
    static const TriangleCreator iTriangleCreator;
    static const WavefrontOBJCreator iWavefrontOBJCreator;
    rscMgr.AddCreator(&iTriangleCreator);
    rscMgr.AddCreator(&iWavefrontOBJCreator);
}

std::shared_ptr<GeometryInstance> TriangleCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<TriangleInstance>(GetName(), std::move(name));
}

std::shared_ptr<GeometryInstance> WavefrontOBJCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<WavefrontOBJInstance>(GetName(), std::move(name));
}
