#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>

void RegisterFresnelCreators(ResourceManager &rscMgr);

class AlwaysOneFresnelCreator : public FresnelCreator
{
public:

    AlwaysOneFresnelCreator() : FresnelCreator("AlwaysOne") { }

    std::shared_ptr<FresnelInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class FresnelConductorCreator : public FresnelCreator
{
public:

    FresnelConductorCreator() : FresnelCreator("Conductor") { }

    std::shared_ptr<FresnelInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class FresnelDielectricCreator : public FresnelCreator
{
public:

    FresnelDielectricCreator() : FresnelCreator("Dielectric") { }

    std::shared_ptr<FresnelInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class FresnelSchlickCreator : public FresnelCreator
{
public:

    FresnelSchlickCreator() : FresnelCreator("Schlick") { }

    std::shared_ptr<FresnelInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};
