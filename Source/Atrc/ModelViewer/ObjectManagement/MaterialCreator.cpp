#include <Atrc/ModelViewer/ObjectManagement/MaterialCreator.h>

namespace
{
    class IdealBlackInstance : public MaterialInstance
    {
    public:

        using MaterialInstance::MaterialInstance;

        void Display(ObjectManager&) override { }
    };

    class IdealDiffuseInstance : public MaterialInstance
    {
        TextureSlot albedo_;

    public:

        using MaterialInstance::MaterialInstance;

        void Display(ObjectManager &objMgr) override
        {
            if(ImGui::TreeNode("albedo"))
            {
                albedo_.Display(objMgr);
                ImGui::TreePop();
            }
        }
    };

    class IdealMirrorInstance : public MaterialInstance
    {
        TextureSlot rc_;
        FresnelSlot fresnel_;

    public:

        using MaterialInstance::MaterialInstance;

        void Display(ObjectManager& objMgr) override
        {
            if(ImGui::TreeNode("rc"))
            {
                rc_.Display(objMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("fresnel"))
            {
                fresnel_.Display(objMgr);
                ImGui::TreePop();
            }
        }
    };
}

void RegisterMaterialCreators(ObjectManager &objMgr)
{
    static const IdealBlackCreator iIdealBlackCreator;
    static const IdealDiffuseCreator iIdealDiffuseCreator;
    static const IdealMirrorCreator iIdealMirrorCreator;
    objMgr.AddCreator(&iIdealBlackCreator);
    objMgr.AddCreator(&iIdealDiffuseCreator);
    objMgr.AddCreator(&iIdealMirrorCreator);
}

std::shared_ptr<MaterialInstance> IdealBlackCreator::Create(std::string name) const
{
    return std::make_shared<IdealBlackInstance>(std::move(name));
}

std::shared_ptr<MaterialInstance> IdealDiffuseCreator::Create(std::string name) const
{
    return std::make_shared<IdealDiffuseInstance>(std::move(name));
}

std::shared_ptr<MaterialInstance> IdealMirrorCreator::Create(std::string name) const
{
    return std::make_shared<IdealMirrorInstance>(std::move(name));
}
