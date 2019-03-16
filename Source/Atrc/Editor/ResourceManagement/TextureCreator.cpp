#include <Atrc/Editor/ResourceManagement/TextureCreator.h>
#include <Atrc/Editor/FilenameSlot.h>
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
        TFilenameSlot<true> filenameSlot_;

    protected:

        void Export(const ResourceManager &rscMgr, SceneExportingContext &ctx) const override
        {
            ctx.AddLine("type = Image;");
            ctx.AddLine("filename = \"", filenameSlot_.GetExportedFilename(ctx), "\";");
        }
        
    public:

        using TextureInstance::TextureInstance;

        void Display([[maybe_unused]] ResourceManager &rscMgr) override
        {
            static FileBrowser fileBrowser("browse image filename", false, "");
            filenameSlot_.Display(fileBrowser);
        }

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            filenameSlot_.Import(params["filename"].AsValue(), ctx);
        }
    };

    class HDRTextureInstance : public TextureInstance
    {
        TFilenameSlot<true> filenameSlot_;

    protected:

        void Export(const ResourceManager &rscMgr, SceneExportingContext &ctx) const override
        {
            ctx.AddLine("type = HDR;");
            ctx.AddLine("filename = \"", filenameSlot_.GetExportedFilename(ctx), "\";");
        }

    public:

        using TextureInstance::TextureInstance;

        void Display([[maybe_unused]] ResourceManager &rscMgr) override
        {
            static FileBrowser fileBrowser("browse image filename", false, "");
            filenameSlot_.Display(fileBrowser);
        }

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            filenameSlot_.Import(params["filename"].AsValue(), ctx);
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
