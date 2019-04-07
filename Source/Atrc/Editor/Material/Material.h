#pragma once

#include "Atrc/Editor/SH2DScriptExporter.h"

class MaterialPool;
class TexturePool;

class IMaterial : public IResource, public ExportAsConfigGroup
{
public:

    using IResource::IResource;

    virtual std::string Save() const = 0;

    virtual std::string Load(AGZ::RefList<MaterialPool, TexturePool> pools) = 0;

    virtual std::string Export() const = 0;

    virtual void Display() = 0;

    virtual bool IsMultiline() const noexcept = 0;

    virtual const MaterialPool *GetPool() const noexcept = 0;
};

DEFINE_DEFAULT_RESOURCE_CREATOR_INTERFACE(IMaterial);

void RegisterBuiltinMaterialCreators(MaterialFactory &factory);
