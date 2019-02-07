#include <AGZUtils/Utils/Mesh.h>
#include <Atrc/ModelViewer/ResourceManagement/GeometryCreator.h>
#include <Atrc/ModelViewer/FileBrowser.h>

namespace
{
    class WavefrontOBJInstance : public GeometryInstance
    {
        std::shared_ptr<GL::VertexBuffer<Vertex>> vtxBuf_;
        AGZ::Mesh::GeometryMeshGroup<float> meshGroup_;
        std::string filename_;

        void BuildVertexBufferFromMeshGroup()
        {
            std::shared_ptr<GL::VertexBuffer<Vertex>> buf = std::make_shared<GL::VertexBuffer<Vertex>>();
            std::vector<Vertex> vtxData;

            for(auto &it : meshGroup_.submeshes)
            {
                for(auto &v : it.second.vertices)
                {
                    Vertex nv;
                    nv.pos = v.pos.xzy();
                    nv.nor = v.nor.xzy();
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
            filename_ = "";
        }

        void LoadWavefrontOBJ(std::string filename)
        {
            Clear();

            AGZ::Mesh::WavefrontObj<float> obj;
            if(!obj.LoadFromFile(filename))
            {
                Clear();
                return;
            }

            meshGroup_ = obj.ToGeometryMeshGroup();
            obj.Clear();

            filename_ = std::move(filename);
            BuildVertexBufferFromMeshGroup();
        }

    public:

        using GeometryInstance::GeometryInstance;

        std::shared_ptr<const GL::VertexBuffer<Vertex>> GetVertexBuffer() const override
        {
            return vtxBuf_;
        }

        void Display(ResourceManager&) override
        {
            ImGui::TextWrapped("filename: %s", filename_.c_str());

            bool clickLoadPopup = false;
            if(ImGui::Button("load##load_data_from_file"))
                clickLoadPopup = true;

            if(clickLoadPopup)
                ImGui::OpenPopup("load data from .obj");
            if(ImGui::BeginPopup("load data from .obj", ImGuiWindowFlags_AlwaysAutoResize))
            {
                AGZ::ScopeGuard popupExitGuard([] { ImGui::EndPopup(); });
                
                static FileBrowser fileBrowser;
                if(clickLoadPopup)
                {
                    fileBrowser.SetLabel("file browser");
                    fileBrowser.SetTarget(false);
                    fileBrowser.SetCurrentDirectory();
                }

                ImGui::Text("%s", fileBrowser.GetResult().c_str());

                ImGui::SameLine();

                if(ImGui::Button("browse"))
                    ImGui::OpenPopup("file browser");
                fileBrowser.Display();

                if(ImGui::Button("ok"))
                {
                    LoadWavefrontOBJ(fileBrowser.GetResult());
                    ImGui::CloseCurrentPopup();
                }
                else if(ImGui::SameLine(); ImGui::Button("cancel"))
                    ImGui::CloseCurrentPopup();
            }
        }

        std::vector<std::string> GetSubmeshNames() const override
        {
            std::vector<std::string> ret;
            std::transform(begin(meshGroup_.submeshes), end(meshGroup_.submeshes), std::back_inserter(ret),
                [](auto &it) { return it.first; });
            return ret;
        }
    };
}

void RegisterGeometryCreators(ResourceManager &rscMgr)
{
    static const WavefrontOBJCreator iWavefrontOBJCreator;
    rscMgr.AddCreator(&iWavefrontOBJCreator);
}

std::shared_ptr<GeometryInstance> WavefrontOBJCreator::Create(std::string name) const
{
    return std::make_shared<WavefrontOBJInstance>(std::move(name));
}
