#pragma once

#include <Atrc/ModelViewer/ObjectManagement/ObjectManager.h>

void RegisterMaterialCreators(ObjectManager &objMgr);

class IdealBlackCreator : public MaterialCreator
{
public:

    IdealBlackCreator() : MaterialCreator("black") { }

    std::shared_ptr<MaterialInstance> Create(std::string name) const override;
};

class IdealDiffuseCreator : public MaterialCreator
{
public:

    IdealDiffuseCreator() : MaterialCreator("diffuse") { }

    std::shared_ptr<MaterialInstance> Create(std::string name) const override;
};

class IdealMirrorCreator : public MaterialCreator
{
public:

    IdealMirrorCreator() : MaterialCreator("mirror") { }

    std::shared_ptr<MaterialInstance> Create(std::string name) const override;
};
