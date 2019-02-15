#include <Atrc/Editor/ResourceManagement/TextureCreator.h>
#include <Atrc/Editor/FilenameSlot.h>

namespace
{
    class ConstantTextureInstance : public TextureInstance
    {
        Vec3f texel_;

    public:

        using TextureInstance::TextureInstance;

        void Display([[maybe_unused]] ResourceManager &rscMgr) override
        {
            ImGui::ColorEdit3("texel", &texel_[0], ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
        }

        void Export(const ResourceManager&, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = Constant;");
            ctx.AddLine("texel = ", AGZ::To<char>(texel_), ";");
        }
    };

    class Constant1TextureInstance : public TextureInstance
    {
        float texel_ = 0;

    public:

        using TextureInstance::TextureInstance;

        void Display([[maybe_unused]] ResourceManager &rscMgr) override
        {
            ImGui::InputFloat("texel", &texel_, 0.01f, 0.1f, 7);
        }

        void Export(const ResourceManager&, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = Constant1;");
            ctx.AddLine("texel = ", texel_, ";");
        }
    };

    class ImageTextureInstance : public TextureInstance
    {
        TFilenameSlot<true> filenameSlot_;
        
    public:

        using TextureInstance::TextureInstance;

        void Display([[maybe_unused]] ResourceManager &rscMgr) override
        {
            static FileBrowser fileBrowser("browse image filename", false, "");
            filenameSlot_.Display(fileBrowser);
        }

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = Image;");
            ctx.AddLine("filename = \"", filenameSlot_.GetExportedFilename(ctx), "\";");
        }
    };
}

void RegisterTextureCreators(ResourceManager &rscMgr)
{
    static const ConstantTextureCreator iConstantTextureCreator;
    static const Constant1TextureCreator iConstant1TextureCreator;
    static const ImageTextureCreator iImageTextureCreator;
    rscMgr.AddCreator(&iConstantTextureCreator);
    rscMgr.AddCreator(&iConstant1TextureCreator);
    rscMgr.AddCreator(&iImageTextureCreator);
}

std::shared_ptr<TextureInstance> ConstantTextureCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<ConstantTextureInstance>(std::move(name));
}

std::shared_ptr<TextureInstance> Constant1TextureCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<Constant1TextureInstance>(std::move(name));
}

std::shared_ptr<TextureInstance> ImageTextureCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<ImageTextureInstance>(std::move(name));
}
