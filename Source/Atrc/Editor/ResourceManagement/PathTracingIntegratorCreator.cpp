#include <Atrc/Editor/ResourceManagement/PathTracingIntegratorCreator.h>

namespace
{
    class FullPathTracingIntegratorInstance : public PathTracingIntegratorInstance
    {
        int minDepth_ = 5;
        int maxDepth_ = 50;
        float contProb_ = 0.9f;
        bool sampleAllLights_ = true;

    public:

        using PathTracingIntegratorInstance::PathTracingIntegratorInstance;

        void Display(ResourceManager&) override
        {
            ImGui::InputInt("min depth", &minDepth_, 0);
            ImGui::InputInt("max depth", &maxDepth_, 0);
            ImGui::InputFloat("cont prob", &contProb_);
            ImGui::Checkbox("sample all lights", &sampleAllLights_);
        }

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = Full;");
            ctx.AddLine("minDepth = ", minDepth_, ";");
            ctx.AddLine("maxDepth = ", maxDepth_, ";");
            ctx.AddLine("contProb = ", contProb_, ";");
            ctx.AddLine("sampleAllLights = ", sampleAllLights_ ? "True" : "False", ";");
        }
    };
}

void RegisterPathTracingIntegratorCreators(ResourceManager &rscMgr)
{
    static const FullPathTracingIntegratorCreator iFullPathTracingIntegratorCreator;
    rscMgr.AddCreator(&iFullPathTracingIntegratorCreator);
}

std::shared_ptr<PathTracingIntegratorInstance> FullPathTracingIntegratorCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<FullPathTracingIntegratorInstance>(std::move(name));
}
