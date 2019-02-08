#include <sstream>

#include <Atrc/ModelViewer/ResourceManagement/TextureCreator.h>
#include <Atrc/ModelViewer/FilenameSlot.h>

namespace
{
    class ConstantTextureInstance : public TextureInstance
    {
        Vec3f texel_;

    public:

        using TextureInstance::TextureInstance;

        void Display([[maybe_unused]] ResourceManager &rscMgr) override
        {
            ImGui::ColorEdit3("texel", &texel_[0]);
        }

        void Export(std::stringstream &sst, const ResourceManager&, ExportingContext &ctx) const override
        {
            sst << ctx.Indent() << "type = Constant;" << std::endl;
            sst << ctx.Indent() << "texel = " << AGZ::To<char>(texel_) << ";" << std::endl;
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

        void Export(std::stringstream &sst, const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            sst << ctx.Indent() << "type = image;\n";
            sst << AGZ::TFormatter<char>("{}filename = {};\n").Arg(ctx.Indent(), filenameSlot_.GetExportedFilename(ctx));
        }
    };
}

void RegisterTextureCreators(ResourceManager &rscMgr)
{
    static const ConstantTextureCreator iConstantTextureCreator;
    static const ImageTextureCreator iImageTextureCreator;
    rscMgr.AddCreator(&iConstantTextureCreator);
    rscMgr.AddCreator(&iImageTextureCreator);
}

std::shared_ptr<TextureInstance> ConstantTextureCreator::Create(std::string name) const
{
    return std::make_shared<ConstantTextureInstance>(std::move(name));
}

std::shared_ptr<TextureInstance> ImageTextureCreator::Create(std::string name) const
{
    return std::make_shared<ImageTextureInstance>(std::move(name));
}
