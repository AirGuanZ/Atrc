#include <Atrc/ModelViewer/ObjectManagement/MaterialCreator.h>

void RegisterMaterialCreators(ObjectManager &objMgr)
{
    static const IdealDiffuseCreator iIdealDiffuseCreator;
    objMgr.AddCreator(&iIdealDiffuseCreator);
}

void IdealDiffuseInstance::Display()
{
    ImGui::ColorEdit3("albedo", &albedo_[0]);
}

IdealDiffuseCreator::IdealDiffuseCreator()
    : MaterialCreator("ideal diffuse")
{
    
}

std::shared_ptr<MaterialInstance> IdealDiffuseCreator::Create(std::string name) const
{
    return std::make_shared<IdealDiffuseInstance>(std::move(name));
}
