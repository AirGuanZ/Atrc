#pragma once

#include <Atrc/ModelViewer/ObjectManagement/ObjectManager.h>

void RegisterMaterialCreators(ObjectManager &objMgr);

class IdealDiffuseCreator : public MaterialCreator
{
public:

    IdealDiffuseCreator() : MaterialCreator("ideal diffuse") { }

    std::shared_ptr<MaterialInstance> Create(std::string name) const override;
};
