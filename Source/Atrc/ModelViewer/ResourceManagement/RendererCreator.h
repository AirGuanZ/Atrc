#pragma once

#include <Atrc/ModelViewer/ResourceManagement/ResourceManager.h>

void RegisterRendererCreators(ResourceManager &rscMgr);

class PathTracingRendererCreator : public RendererCreator
{
public:

    PathTracingRendererCreator() : RendererCreator("pt") { }

    std::shared_ptr<RendererInstance> Create(std::string name) const override;
};
