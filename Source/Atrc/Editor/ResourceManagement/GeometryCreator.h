#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>

void RegisterGeometryCreators(ResourceManager &rscMgr);

class TriangleCreator : public GeometryCreator
{
public:

    TriangleCreator() : GeometryCreator("triangle") { }

    std::shared_ptr<GeometryInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class WavefrontOBJCreator : public GeometryCreator
{
public:

    WavefrontOBJCreator() : GeometryCreator("wavefront") { }

    std::shared_ptr<GeometryInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};
