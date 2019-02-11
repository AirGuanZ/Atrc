#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>

void RegisterFresnelCreators(ResourceManager &rscMgr);

class FresnelConductorCreator : public FresnelCreator
{
public:

    FresnelConductorCreator() : FresnelCreator("conductor") { }

    std::shared_ptr<FresnelInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class FresnelDielectricCreator : public FresnelCreator
{
public:

    FresnelDielectricCreator() : FresnelCreator("dielectric") { }

    std::shared_ptr<FresnelInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class FresnelSchlickCreator : public FresnelCreator
{
public:

    FresnelSchlickCreator() : FresnelCreator("schlick") { }

    std::shared_ptr<FresnelInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};
