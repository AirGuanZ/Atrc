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

    class MISPathTracingIntegratorInstance : public PathTracingIntegratorInstance
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

    class NativePathTracingIntegratorInstance : public PathTracingIntegratorInstance
    {
        int minDepth_ = 5;
        int maxDepth_ = 50;
        float contProb_ = 0.9f;

    public:

        using PathTracingIntegratorInstance::PathTracingIntegratorInstance;

        void Display(ResourceManager&) override
        {
            ImGui::InputInt("min depth", &minDepth_, 0);
            ImGui::InputInt("max depth", &maxDepth_, 0);
            ImGui::InputFloat("cont prob", &contProb_);
        }

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = Native;");
            ctx.AddLine("minDepth = ", minDepth_, ";");
            ctx.AddLine("maxDepth = ", maxDepth_, ";");
            ctx.AddLine("contProb = ", contProb_, ";");
        }
    };
}

void RegisterPathTracingIntegratorCreators(ResourceManager &rscMgr)
{
    static const FullPathTracingIntegratorCreator iFullPathTracingIntegratorCreator;
    static const MISPathTracingIntegratorCreator iMISPathTracingIntegratorCreator;
    static const NativePathTracingIntegratorCreator iNativePathTracingIntegratorCreator;
    rscMgr.AddCreator(&iFullPathTracingIntegratorCreator);
    rscMgr.AddCreator(&iMISPathTracingIntegratorCreator);
    rscMgr.AddCreator(&iNativePathTracingIntegratorCreator);
}

std::shared_ptr<PathTracingIntegratorInstance> FullPathTracingIntegratorCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<FullPathTracingIntegratorInstance>(std::move(name));
}

std::shared_ptr<PathTracingIntegratorInstance> MISPathTracingIntegratorCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<MISPathTracingIntegratorInstance>(std::move(name));
}

std::shared_ptr<PathTracingIntegratorInstance> NativePathTracingIntegratorCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<NativePathTracingIntegratorInstance>(std::move(name));
}
