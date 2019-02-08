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

        void Export(const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            ctx.AddLine("type = Conductor;");
            ctx.AddLine("etaI = ", AGZ::To<char>(etaI_), ";");
            ctx.AddLine("etaT = ", AGZ::To<char>(etaT_), ";");
            ctx.AddLine("k = ", AGZ::To<char>(k_), ";");
        }
    };

    class FresnelDielectricInstance : public FresnelInstance
    {
    protected:

        float etaI_ = 0, etaT_ = 0;

    public:

        using FresnelInstance::FresnelInstance;

        void Display(ResourceManager&) override
        {
            ImGui::InputFloat("etaI", &etaI_);
            ImGui::InputFloat("etaT", &etaT_);
        }

        void Export(const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            ctx.AddLine("type = Dielectric;");
            ctx.AddLine("etaI = ", std::to_string(etaI_), ";");
            ctx.AddLine("etaT = ", std::to_string(etaT_), ";");
        }
    };

    class FresnelSchlickInstance : public FresnelDielectricInstance
    {
    public:

        using FresnelDielectricInstance::FresnelDielectricInstance;

        void Export(const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            ctx.AddLine("type = Schlick;");
            ctx.AddLine("etaI = ", std::to_string(etaI_), ";");
            ctx.AddLine("etaT = ", std::to_string(etaT_), ";");
        }
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
