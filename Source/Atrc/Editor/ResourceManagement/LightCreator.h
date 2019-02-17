#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>

void RegisterLightCreators(ResourceManager &rscMgr);

class SkyLightCreator : public LightCreator
{
public:

    SkyLightCreator() : LightCreator("Sky") { }

    std::shared_ptr<LightInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};
