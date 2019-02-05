#pragma once

#include <Atrc/ModelViewer/ObjectManagement/ObjectManager.h>

void RegisterMaterialCreators(ObjectManager &objMgr);

class IdealDiffuseInstance : public MaterialInstance
{
    Vec3f albedo_;

public:

    using MaterialInstance::MaterialInstance;

    void Display() override;
};

class IdealDiffuseCreator : public MaterialCreator
{
public:

    IdealDiffuseCreator();

    std::shared_ptr<MaterialInstance> Create(std::string name) const override;
};
