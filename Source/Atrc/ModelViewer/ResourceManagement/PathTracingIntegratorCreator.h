#pragma once

#include <Atrc/ModelViewer/ResourceManagement/ResourceManager.h>

void RegisterPathTracingIntegratorCreators(ResourceManager &rscMgr);

class FullPathTracingIntegratorCreator : public PathTracingIntegratorCreator
{
public:

    FullPathTracingIntegratorCreator() : PathTracingIntegratorCreator("full") { }

    std::shared_ptr<PathTracingIntegratorInstance> Create(std::string name) const override;
};
