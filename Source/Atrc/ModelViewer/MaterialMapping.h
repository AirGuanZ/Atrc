#pragma once

#include <Atrc/ModelViewer/ResourceManagement/ResourceManager.h>

class SingleMaterialMapping
{
    MaterialSlot slot_;

public:

    void Display(ResourceManager &rscMgr)
    {
        slot_.Display(rscMgr);
    }

    void Export(std::stringstream &sst, const ResourceManager &rscMgr, ExportingContext &ctx) const
    {
        slot_.Export(sst, rscMgr, ctx);
    }
};
