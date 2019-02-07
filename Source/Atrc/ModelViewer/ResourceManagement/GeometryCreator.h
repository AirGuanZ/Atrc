#pragma once

#include <Atrc/ModelViewer/ResourceManagement/ResourceManager.h>

void RegisterGeometryCreators(ResourceManager &rscMgr);

class WavefrontOBJCreator : public GeometryCreator
{
public:

    WavefrontOBJCreator() : GeometryCreator("wavefront") { }

    std::shared_ptr<GeometryInstance> Create(std::string name) const override;
};
