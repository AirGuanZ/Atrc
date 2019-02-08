#include <Atrc/ModelViewer/ResourceManagement/PathTracingIntegratorCreator.h>

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
            ImGui::InputInt("min depth", &minDepth_);
            ImGui::InputInt("max depth", &maxDepth_);
            ImGui::InputFloat("cont prob", &contProb_);
            ImGui::Checkbox("sample all lights", &sampleAllLights_);
        }

        void Export(std::stringstream &sst, const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            sst << AGZ::TFormatter<char>("{}type = Full;\n").Arg(ctx.Indent());
            sst << AGZ::TFormatter<char>("{}minDepth = {};\n").Arg(ctx.Indent(), minDepth_);
            sst << AGZ::TFormatter<char>("{}maxDepth = {};\n").Arg(ctx.Indent(), maxDepth_);
            sst << AGZ::TFormatter<char>("{}contProb = {};\n").Arg(ctx.Indent(), contProb_);
            sst << AGZ::TFormatter<char>("{}sampleAllLights = {};\n").Arg(ctx.Indent(), sampleAllLights_ ? "True" : "False");
        }
    };
}

void RegisterPathTracingIntegratorCreators(ResourceManager &rscMgr)
{
    static const FullPathTracingIntegratorCreator iFullPathTracingIntegratorCreator;
    rscMgr.AddCreator(&iFullPathTracingIntegratorCreator);
}

std::shared_ptr<PathTracingIntegratorInstance> FullPathTracingIntegratorCreator::Create(std::string name) const
{
    return std::make_shared<FullPathTracingIntegratorInstance>(std::move(name));
}
