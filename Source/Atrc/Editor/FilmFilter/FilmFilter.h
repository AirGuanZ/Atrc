#pragma once

#include <memory>

#include <AGZUtils/Utils/Config.h>
#include <Atrc/Editor/ResourceInstance/ResourceInstance.h>

class IFilmFilter : public IResource, public ExportAsConfigGroup<>
{
public:

    using IResource::IResource;

    virtual std::string Save() const = 0;

    virtual void Load(const AGZ::ConfigGroup &params) = 0;

    virtual void Display() = 0;

    virtual bool IsMultiline() const noexcept = 0;
};

class IFilmFilterCreator : public IResourceCreator
{
public:

    using Resource = IFilmFilter;

    using IResourceCreator::IResourceCreator;

    virtual std::shared_ptr<IFilmFilter> Create() const = 0;
};

void RegisterBuiltinFilmFilterCreators(ResourceFactory<IFilmFilterCreator> &factory);
