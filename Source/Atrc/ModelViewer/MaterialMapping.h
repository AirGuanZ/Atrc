#pragma once

#include <Atrc/ModelViewer/ResourceManagement/ResourceManager.h>

class MaterialMapping
{
public:

    virtual ~MaterialMapping() = default;

    virtual void Display(ResourceManager &rscMgr) = 0;
};

class MaterialMappingSelector
{
    std::unique_ptr<MaterialMapping> mapping_;
    const std::string *curMappingTypeName_ = nullptr;

public:

    void Display(ResourceManager &rscMgr);
};

class SingleMaterialMapping : public MaterialMapping
{
    MaterialSlot slot_;

public:

    void Display(ResourceManager &rscMgr) override;
};
