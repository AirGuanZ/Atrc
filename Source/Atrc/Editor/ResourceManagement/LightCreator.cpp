#include <Atrc/Editor/ResourceManagement/LightCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace
{
    class EnvironmentLightInstance : public LightInstance
    {
        TextureSlot tex_;
        Deg rotateDeg_;

    protected:

        void Export(const ResourceManager &rscMgr, SceneExportingContext &ctx) const override
        {
            ctx.AddLine("type = Environment;");
            ExportSubResource("tex", rscMgr, ctx, tex_);
            ctx.AddLine("transform = (RotateY(Deg(", rotateDeg_.value, ")));");
            ctx.AddLine("rotateYDeg = ", rotateDeg_.value, ";");
        }

    public:

        using LightInstance::LightInstance;

        void Display(ResourceManager &rscMgr) override
        {
            tex_.Display(rscMgr);
            ImGui::InputFloat("rotate angle", &rotateDeg_.value);
        }

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            tex_.SetInstance(GetResourceInstance<TextureInstance>(rscMgr, root, params["tex"], ctx));
            rotateDeg_.value = params["rotateYDeg"].Parse<float>();
        }
    };

    class SkyLightInstance : public LightInstance
    {
        Vec3f top_ = Vec3f(1);
        Vec3f bottom_ = Vec3f(1);

    protected:

        void Export(const ResourceManager&, SceneExportingContext &ctx) const override
        {
            ctx.AddLine("type = Sky;");
            ctx.AddLine("top = ", AGZ::To<char>(top_), ";");
            ctx.AddLine("bottom = ", AGZ::To<char>(bottom_), ";");
        }

    public:

        using LightInstance::LightInstance;

        void Display(ResourceManager&) override
        {
            ImGui::ColorEdit3("top", &top_[0], ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
            ImGui::ColorEdit3("bottom", &bottom_[0], ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
        }

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            top_ = Atrc::Mgr::Parser::TFloat::ParseSpectrum<float>(params["top"]);
            bottom_ = Atrc::Mgr::Parser::TFloat::ParseSpectrum<float>(params["bottom"]);
        }
    };
}

void RegisterLightCreators(ResourceManager &rscMgr)
{
    static const EnvironmentLightCreator iEnvironmentLightCreator;
    static const SkyLightCreator iSkyLightCreator;
    rscMgr.AddCreator(&iEnvironmentLightCreator);
    rscMgr.AddCreator(&iSkyLightCreator);
}

std::shared_ptr<LightInstance> EnvironmentLightCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<EnvironmentLightInstance>(GetName(), std::move(name));
}

std::shared_ptr<LightInstance> SkyLightCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<SkyLightInstance>(GetName(), std::move(name));
}
