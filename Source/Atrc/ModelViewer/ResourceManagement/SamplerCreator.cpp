#include <Atrc/ModelViewer/ResourceManagement/SamplerCreator.h>

namespace
{
    class NativeSamplerInstance : public SamplerInstance
    {
        bool hasSeed_ = false;
        int seed_ = 42;
        int spp_ = 100;

    public:

        using SamplerInstance::SamplerInstance;

        void Display(ResourceManager&) override
        {
            ImGui::Checkbox("with seed", &hasSeed_);
            if(hasSeed_)
                ImGui::InputInt("seed", &seed_);
            ImGui::InputInt("spp", &spp_);
        }
    };
}

void RegisterSamplerCreators(ResourceManager &rscMgr)
{
    static const NativeSamplerCreator iNativeSamplerCreator;
    rscMgr.AddCreator(&iNativeSamplerCreator);
}

std::shared_ptr<SamplerInstance> NativeSamplerCreator::Create(std::string name) const
{
    return std::make_shared<NativeSamplerInstance>(std::move(name));
}
