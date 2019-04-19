#include <Atrc/Editor/ResourceManagement/TextureCreator.h>
#include <Atrc/Editor/FileSelector.h>
#include <Atrc/Mgr/Parser.h>

namespace
{
    class ConstantTextureInstance : public TextureInstance
    {
        Vec3f texel_;

    protected:

        void Export(const ResourceManager&, SceneExportingContext &ctx) const override
        {
            ctx.AddLine("type = Constant;");
            ctx.AddLine("texel = ", AGZ::To<char>(texel_), ";");
        }

    public:

        using TextureInstance::TextureInstance;

        void Display([[maybe_unused]] ResourceManager &rscMgr) override
        {
            ImGui::ColorEdit3("texel", &texel_[0], ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
        }

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            texel_ = Atrc::Mgr::Parser::TFloat::ParseVec3<float>(params["texel"]);
        }
    };

    class Constant1TextureInstance : public TextureInstance
    {
        float texel_ = 0;

    protected:

        void Export(const ResourceManager&, SceneExportingContext &ctx) const override
        {
            ctx.AddLine("type = Constant1;");
            ctx.AddLine("texel = ", texel_, ";");
        }

    public:

        using TextureInstance::TextureInstance;

        void Display([[maybe_unused]] ResourceManager &rscMgr) override
        {
            ImGui::InputFloat("texel", &texel_, 0.01f, 0.1f, 7);
        }

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            texel_ = params["texel"].Parse<float>();
        }
    };

    class ImageTextureInstance : public TextureInstance
    {
        FileSelector filename_;
        bool reverseV_ = false;

    protected:

        void Export(const ResourceManager &rscMgr, SceneExportingContext &ctx) const override
        {
            ctx.AddLine("type = Image;");
            ctx.AddLine("filename = \"", filename_.RelativeTo(ctx.workspaceDirectory).string(), "\";");
            ctx.AddLine("reverseV = ", reverseV_ ? "True" : "False", ";");
        }
        
    public:

        using TextureInstance::TextureInstance;

        void Display([[maybe_unused]] ResourceManager &rscMgr) override
        {
            ImGui::PushID(0);
            ImGui::Checkbox("", &reverseV_);
            ImGui::PopID();
            ImGui::ShowTooltipForLastItem("flip vertically");
            ImGui::SameLine();
            filename_.Display();
        }

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            filename_.SetFilename(std::filesystem::path(ctx.workspacePath) / params["filename"].AsValue());
            if(auto *n = params.Find("reverseV"))
                reverseV_ = Atrc::Mgr::Parser::ParseBool(*n);
            else
                reverseV_ = false;
        }
    };

    class HDRTextureInstance : public TextureInstance
    {
        FileSelector filename_;

    protected:

        void Export(const ResourceManager &rscMgr, SceneExportingContext &ctx) const override
        {
            ctx.AddLine("type = HDR;");
            ctx.AddLine("filename = \"", filename_.RelativeTo(ctx.workspaceDirectory).string(), "\";");
        }

    public:

        using TextureInstance::TextureInstance;

        void Display([[maybe_unused]] ResourceManager &rscMgr) override
        {
            filename_.Display();
        }

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            filename_.SetFilename(std::filesystem::path(ctx.workspacePath) / params["filename"].AsValue());
        }
    };
}

void RegisterTextureCreators(ResourceManager &rscMgr)
{
    static const ConstantTextureCreator iConstantTextureCreator;
    static const Constant1TextureCreator iConstant1TextureCreator;
    static const ImageTextureCreator iImageTextureCreator;
    static const HDRTextureCreator iHDRTextureCreator;
    rscMgr.AddCreator(&iConstantTextureCreator);
    rscMgr.AddCreator(&iConstant1TextureCreator);
    rscMgr.AddCreator(&iImageTextureCreator);
    rscMgr.AddCreator(&iHDRTextureCreator);
}

std::shared_ptr<TextureInstance> ConstantTextureCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<ConstantTextureInstance>(GetName(), std::move(name));
}

std::shared_ptr<TextureInstance> Constant1TextureCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<Constant1TextureInstance>(GetName(), std::move(name));
}

std::shared_ptr<TextureInstance> ImageTextureCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<ImageTextureInstance>(GetName(), std::move(name));
}

std::shared_ptr<TextureInstance> HDRTextureCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<HDRTextureInstance>(GetName(), std::move(name));
}
