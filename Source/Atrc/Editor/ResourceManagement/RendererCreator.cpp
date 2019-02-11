#include <Atrc/Editor/ResourceManagement/RendererCreator.h>

namespace
{
    class PathTracingRendererInstance : public RendererInstance
    {
        int workerCount_ = -1;
        int taskGridSize_ = 16;
        PathTracingIntegratorSlot integratorSlot_;

    public:

        using RendererInstance::RendererInstance;

        void Display(ResourceManager &rscMgr) override
        {
            ImGui::InputInt("worker count", &workerCount_, 0);
            ImGui::InputInt("task size", &taskGridSize_, 0);
            if(ImGui::TreeNodeEx("integrator", ImGuiTreeNodeFlags_DefaultOpen))
            {
                integratorSlot_.Display(rscMgr);
                ImGui::TreePop();
            }
        }

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = PathTracing;");
            ExportSubResource("integrator", rscMgr, ctx, integratorSlot_);
            ctx.AddLine("workerCount = ", workerCount_, ";");
            ctx.AddLine("taskGridSize = ", taskGridSize_, ";");
        }
    };
}

void RegisterRendererCreators(ResourceManager &rscMgr)
{
    static const PathTracingRendererCreator iPathTracingRendererCreator;
    rscMgr.AddCreator(&iPathTracingRendererCreator);
}

std::shared_ptr<RendererInstance> PathTracingRendererCreator::Create(std::string name) const
{
    return std::make_shared<PathTracingRendererInstance>(std::move(name));
}
