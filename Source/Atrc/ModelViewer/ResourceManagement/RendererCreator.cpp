#include <Atrc/ModelViewer/ResourceManagement/RendererCreator.h>

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
            ImGui::InputInt("worker count", &workerCount_);
            ImGui::InputInt("task size", &taskGridSize_);
            if(ImGui::TreeNodeEx("integrator", ImGuiTreeNodeFlags_DefaultOpen))
            {
                integratorSlot_.Display(rscMgr);
                ImGui::TreePop();
            }
        }

        void Export(std::stringstream &sst, const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            sst << ctx.Indent() << "type = PathTracing;\n";
            ExportSubResource("integrator", sst, rscMgr, ctx, integratorSlot_);
            sst << AGZ::TFormatter<char>("{}workerCount = {};\n").Arg(ctx.Indent(), workerCount_);
            sst << AGZ::TFormatter<char>("{}taskGridSize = {};\n").Arg(ctx.Indent(), taskGridSize_);
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
