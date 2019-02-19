#include <Atrc/Editor/ResourceManagement/SamplerCreator.h>

namespace
{
    class NativeSamplerInstance : public SamplerInstance
    {
        bool hasSeed_ = false;
        int seed_ = 42;
        int spp_ = 100;

    protected:

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = Native;");
            ctx.AddLine("spp = ", spp_, ";");
            if(hasSeed_)
                ctx.AddLine("seed = ", seed_, ";");
        }

    public:

        using SamplerInstance::SamplerInstance;

        void Display(ResourceManager&) override
        {
            ImGui::Checkbox("with seed", &hasSeed_);
            if(hasSeed_)
                ImGui::InputInt("seed", &seed_, 0);
            ImGui::InputInt("spp", &spp_, 0);
        }

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            if(auto nSeed = params.Find("seed"))
            {
                hasSeed_ = true;
                seed_ = nSeed->Parse<int>();
            }
            else
                hasSeed_ = false;
            spp_ = params["spp"].Parse<int>();
        }
    };
}

void RegisterSamplerCreators(ResourceManager &rscMgr)
{
    static const NativeSamplerCreator iNativeSamplerCreator;
    rscMgr.AddCreator(&iNativeSamplerCreator);
}

std::shared_ptr<SamplerInstance> NativeSamplerCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<NativeSamplerInstance>(GetName(), std::move(name));
}
