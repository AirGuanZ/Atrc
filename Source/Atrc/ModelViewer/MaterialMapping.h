#pragma once

#include <Atrc/ModelViewer/ObjectManagement/ObjectManager.h>

class MaterialMapping
{
public:

    virtual ~MaterialMapping() = default;

    virtual void Display(ObjectManager &objMgr) = 0;
};

class MaterialMappingSelector
{
    std::unique_ptr<MaterialMapping> mapping_;
    const std::string *curMappingTypeName_ = nullptr;

public:

    void Display(ObjectManager &objMgr);
};

class SingleMaterialMapping : public MaterialMapping
{
    std::shared_ptr<MaterialInstance> material_;

public:

    void Display(ObjectManager &objMgr) override;
};
