#include <Atrc/ModelViewer/ObjectManagement/TextureCreator.h>
#include <Atrc/ModelViewer/FileBrowser.h>

namespace
{
    class ConstantTextureInstance : public TextureInstance
    {
        Vec3f texel_;

    public:

        using TextureInstance::TextureInstance;

        void Display([[maybe_unused]] ObjectManager &objMgr) override
        {
            ImGui::ColorEdit3("texel", &texel_[0]);
        }
    };

    class ImageTextureInstance : public TextureInstance
    {
        std::string filename_;
        
    public:

        using TextureInstance::TextureInstance;

        void Display([[maybe_unused]] ObjectManager &objMgr) override
        {
            static FileBrowser fileBrowser;

            if(!filename_.empty())
                ImGui::Text("%s", filename_.c_str());

            if(ImGui::Button("browse"))
            {
                ImGui::OpenPopup("browse image texture");
                fileBrowser.SetLabel("browse image texture");
                fileBrowser.SetCurrentDirectory();
                fileBrowser.SetTarget(false);
            }

            if(fileBrowser.Display())
                filename_ = fileBrowser.GetResult();
        }
    };
}

void RegisterTextureCreators(ObjectManager &objMgr)
{
    static const ConstantTextureCreator iConstantTextureCreator;
    static const ImageTextureCreator iImageTextureCreator;
    objMgr.AddCreator(&iConstantTextureCreator);
    objMgr.AddCreator(&iImageTextureCreator);
}

std::shared_ptr<TextureInstance> ConstantTextureCreator::Create(std::string name) const
{
    return std::make_shared<ConstantTextureInstance>(std::move(name));
}

std::shared_ptr<TextureInstance> ImageTextureCreator::Create(std::string name) const
{
    return std::make_shared<ImageTextureInstance>(std::move(name));
}
