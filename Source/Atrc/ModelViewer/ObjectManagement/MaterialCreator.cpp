#include <Atrc/ModelViewer/ObjectManagement/MaterialCreator.h>

namespace
{
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
}

void RegisterMaterialCreators(ObjectManager &objMgr)
{
    static const IdealDiffuseCreator iIdealDiffuseCreator;
    objMgr.AddCreator(&iIdealDiffuseCreator);
}

std::shared_ptr<MaterialInstance> IdealDiffuseCreator::Create(std::string name) const
{
    return std::make_shared<IdealDiffuseInstance>(std::move(name));
}
