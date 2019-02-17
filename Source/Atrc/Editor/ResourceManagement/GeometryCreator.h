#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>

void RegisterGeometryCreators(ResourceManager &rscMgr);

class TriangleCreator : public GeometryCreator
{
public:

    TriangleCreator() : GeometryCreator("Triangle") { }

    std::shared_ptr<GeometryInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class WavefrontOBJCreator : public GeometryCreator
{
public:

    WavefrontOBJCreator() : GeometryCreator("TriangleBVH") { }

    std::shared_ptr<GeometryInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};
