#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>

void RegisterEntityCreators(ResourceManager &rscMgr);

void BeginEntityRendering();

void EndEntityRendering();

class GeometricDiffuseLightCreator : public EntityCreator
{
public:

    GeometricDiffuseLightCreator() : EntityCreator("GeometricDiffuse") { }

    std::shared_ptr<EntityInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class GeometricEntityCreator : public EntityCreator
{
public:

    GeometricEntityCreator() : EntityCreator("Geometric") { }

    std::shared_ptr<EntityInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};
