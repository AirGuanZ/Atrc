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

        void Export(std::stringstream &sst, const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            sst << ctx.Indent() << "type = Conductor;\n";
            sst << ctx.Indent() << "etaI = " << AGZ::To<char>(etaI_) << ";\n";
            sst << ctx.Indent() << "etaT = " << AGZ::To<char>(etaT_) << ";\n";
            sst << ctx.Indent() << "k = " << AGZ::To<char>(k_) << ";\n";
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

        void Export(std::stringstream &sst, const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            sst << ctx.Indent() << "type = Dielectric;\n";
            sst << ctx.Indent() << "etaI = " << AGZ::To<char>(etaI_) << ";\n";
            sst << ctx.Indent() << "etaT = " << AGZ::To<char>(etaT_) << ";\n";
        }
    };

    class FresnelSchlickInstance : public FresnelDielectricInstance
    {
    public:

        using FresnelDielectricInstance::FresnelDielectricInstance;

        void Export(std::stringstream &sst, const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            sst << ctx.Indent() << "type = Schlick;\n";
            sst << ctx.Indent() << "etaI = " << AGZ::To<char>(etaI_) << ";\n";
            sst << ctx.Indent() << "etaT = " << AGZ::To<char>(etaT_) << ";\n";
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
