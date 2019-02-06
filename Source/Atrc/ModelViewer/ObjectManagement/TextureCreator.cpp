#include <Atrc/ModelViewer/ObjectManagement/TextureCreator.h>

namespace
{
    class ConstantTextureInstance : public TextureInstance
    {
        Vec3f texel_;

    public:

        using TextureInstance::TextureInstance;

        void Display() override
        {
            ImGui::ColorEdit3("texel", &texel_[0]);
        }
    };
}

void RegisterTextureCreators(ObjectManager &objMgr)
{
    ConstantTextureCreator iConstantTextureCreator;
    objMgr.AddCreator(&iConstantTextureCreator);
}

std::shared_ptr<TextureInstance> ConstantTextureCreator::Create(std::string name) const
{
    return std::make_shared<ConstantTextureInstance>(std::move(name));
}
