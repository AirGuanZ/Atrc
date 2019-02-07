#include <Atrc/ModelViewer/ResourceManagement/FresnelCreator.h>

namespace
{
    class FresnelConductorInstance : public FresnelInstance
    {
        Vec3f etaI_, etaT_, k_;

    public:

        using FresnelInstance::FresnelInstance;

        void Display(ResourceManager&) override
        {
            ImGui::InputFloat3("etaI", &etaI_[0]);
            ImGui::InputFloat3("etaT", &etaT_[0]);
            ImGui::InputFloat3("k", &k_[0]);
        }
    };

    class FresnelDielectricInstance : public FresnelInstance
    {
        float etaI_ = 0, etaT_ = 0;

    public:

        using FresnelInstance::FresnelInstance;

        void Display(ResourceManager&) override
        {
            ImGui::InputFloat("etaI", &etaI_);
            ImGui::InputFloat("etaT", &etaT_);
        }
    };

    class FresnelSchlickInstance : public FresnelDielectricInstance
    {
    public:

        using FresnelDielectricInstance::FresnelDielectricInstance;
    };
}

void RegisterFresnelCreators(ResourceManager &rscMgr)
{
    static const FresnelConductorCreator iFresnelConductorCreator;
    static const FresnelDielectricCreator iFresnelDielectricCreator;
    static const FresnelSchlickCreator iFresnelSchlickCreator;
    rscMgr.AddCreator(&iFresnelConductorCreator);
    rscMgr.AddCreator(&iFresnelDielectricCreator);
    rscMgr.AddCreator(&iFresnelSchlickCreator);
}

std::shared_ptr<FresnelInstance> FresnelConductorCreator::Create(std::string name) const
{
    return std::make_shared<FresnelConductorInstance>(std::move(name));
}

std::shared_ptr<FresnelInstance> FresnelDielectricCreator::Create(std::string name) const
{
    return std::make_shared<FresnelDielectricInstance>(std::move(name));
}

std::shared_ptr<FresnelInstance> FresnelSchlickCreator::Create(std::string name) const
{
    return std::make_shared<FresnelSchlickInstance>(std::move(name));
}
