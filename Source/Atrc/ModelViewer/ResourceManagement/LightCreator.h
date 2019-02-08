#pragma once

#include <Atrc/ModelViewer/ResourceManagement/ResourceManager.h>

void RegisterLightCreators(ResourceManager &rscMgr);

class SkyLightCreator : public LightCreator
{
public:

    SkyLightCreator() : LightCreator("sky") { }

    std::shared_ptr<LightInstance> Create(std::string name) const override;
};
