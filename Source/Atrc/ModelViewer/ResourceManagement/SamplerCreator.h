#pragma once

#include <Atrc/ModelViewer/ResourceManagement/ResourceManager.h>

void RegisterSamplerCreators(ResourceManager &rscMgr);

class NativeSamplerCreator : public SamplerCreator
{
public:

    NativeSamplerCreator() : SamplerCreator("native") { }

    std::shared_ptr<SamplerInstance> Create(std::string name) const override;
};
