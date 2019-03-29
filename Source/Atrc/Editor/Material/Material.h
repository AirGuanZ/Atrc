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

class IMaterialCreator : public IResourceCreator
{
public:

    using Resource = IMaterial;

    using IResourceCreator::IResourceCreator;

    virtual std::shared_ptr<IFilmFilter> Create(AGZ::RefList<MaterialFactory, TextureFactory> facs) const = 0;
};

void RegisterBuiltinMaterialCreators(MaterialFactory &factory);
