#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>

void RegisterPathTracingIntegratorCreators(ResourceManager &rscMgr);

class FullPathTracingIntegratorCreator : public PathTracingIntegratorCreator
{
public:

    FullPathTracingIntegratorCreator() : PathTracingIntegratorCreator("Full") { }

    std::shared_ptr<PathTracingIntegratorInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class MISPathTracingIntegratorCreator : public PathTracingIntegratorCreator
{
public:

    MISPathTracingIntegratorCreator() : PathTracingIntegratorCreator("MIS") { }

    std::shared_ptr<PathTracingIntegratorInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class NativePathTracingIntegratorCreator : public PathTracingIntegratorCreator
{
public:

    NativePathTracingIntegratorCreator() : PathTracingIntegratorCreator("Native") { }

    std::shared_ptr<PathTracingIntegratorInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};
