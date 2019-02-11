#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>

void RegisterRendererCreators(ResourceManager &rscMgr);

class PathTracingRendererCreator : public RendererCreator
{
public:

    PathTracingRendererCreator() : RendererCreator("path tracing") { }

    std::shared_ptr<RendererInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};
