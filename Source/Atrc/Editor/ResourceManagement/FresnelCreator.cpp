#include <Atrc/Editor/ResourceManagement/FresnelCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace
{
    class FresnelConductorInstance : public FresnelInstance
    {
        Vec3f etaI_, etaT_, k_;

    protected:

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = Conductor;");
            ctx.AddLine("etaI = ", AGZ::To<char>(etaI_), ";");
            ctx.AddLine("etaT = ", AGZ::To<char>(etaT_), ";");
            ctx.AddLine("k = ", AGZ::To<char>(k_), ";");
        }

    public:

        using FresnelInstance::FresnelInstance;

        void Display(ResourceManager&) override
        {
            ImGui::InputFloat3("etaI", &etaI_[0]);
            ImGui::InputFloat3("etaT", &etaT_[0]);
            ImGui::InputFloat3("k", &k_[0]);
        }

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            etaI_ = Atrc::Mgr::Parser::TFloat::ParseSpectrum<float>(params["etaI"]);
            etaT_ = Atrc::Mgr::Parser::TFloat::ParseSpectrum<float>(params["etaT"]);
            k_    = Atrc::Mgr::Parser::TFloat::ParseSpectrum<float>(params["k"]);
        }
    };

    class FresnelDielectricInstance : public FresnelInstance
    {
    protected:

        float etaI_ = 0, etaT_ = 0;

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = Dielectric;");
            ctx.AddLine("etaI = ", std::to_string(etaI_), ";");
            ctx.AddLine("etaT = ", std::to_string(etaT_), ";");
        }

    public:

        using FresnelInstance::FresnelInstance;

        void Display(ResourceManager&) override
        {
            ImGui::InputFloat("etaI", &etaI_);
            ImGui::InputFloat("etaT", &etaT_);
        }

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            etaI_ = params["etaI"].Parse<float>();
            etaT_ = params["etaT"].Parse<float>();
        }
    };

    class FresnelSchlickInstance : public FresnelDielectricInstance
    {
    protected:

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = Schlick;");
            ctx.AddLine("etaI = ", std::to_string(etaI_), ";");
            ctx.AddLine("etaT = ", std::to_string(etaT_), ";");
        }

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

std::shared_ptr<FresnelInstance> FresnelConductorCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<FresnelConductorInstance>(GetName(), std::move(name));
}

std::shared_ptr<FresnelInstance> FresnelDielectricCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<FresnelDielectricInstance>(GetName(), std::move(name));
}

std::shared_ptr<FresnelInstance> FresnelSchlickCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<FresnelSchlickInstance>(GetName(), std::move(name));
}
