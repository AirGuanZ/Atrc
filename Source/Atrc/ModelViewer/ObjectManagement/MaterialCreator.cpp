#include <Atrc/ModelViewer/ObjectManagement/MaterialCreator.h>

namespace
{
    class IdealDiffuseInstance : public MaterialInstance
    {
        Vec3f albedo_;

    public:

        using MaterialInstance::MaterialInstance;

        void Display() override
        {
            ImGui::ColorEdit3("albedo", &albedo_[0]);
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
