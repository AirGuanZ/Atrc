#pragma once

#include <map>
#include <memory>
#include <string>
#include <type_traits>

#include <AGZUtils/Utils/Exception.h>
#include <AGZUtils/Utils/Misc.h>

class ResourceCreatingContext;

class HasName
{
    std::string name_;

public:

    explicit HasName(std::string name) noexcept
        : name_(std::move(name))
    {

    }

    const std::string &GetName() const noexcept
    {
        return name_;
    }
};

class IResource : public HasName
{
    const HasName *creator_;

public:

    IResource(std::string name, const HasName *creator) noexcept
        : HasName(std::move(name)), creator_(creator)
    {
        
    }

    const std::string &GetType() const noexcept
    {
        return creator_->GetName();
    }
};

template<typename TResourceCategory>
class IResourceCreator : public HasName
{
    static_assert(std::is_base_of_v<IResource, TResourceCategory>);

public:

    explicit IResourceCreator(std::string name) noexcept
        : HasName(std::move(name))
    {

    }

    virtual ~IResourceCreator() = default;

    virtual std::shared_ptr<TResourceCategory> Create(std::string name, ResourceCreatingContext &ctx) const = 0;
};

template<typename TResourceCategory>
class ResourceFactory
{
    static_assert(std::is_base_of_v<IResource, TResourceCategory>);

    std::map<std::string, const IResourceCreator<TResourceCategory>*> name2Creator_;

public:

    using Creator = IResourceCreator<TResourceCategory>;

    void AddCreator(const Creator *creator)
    {
        AGZ_HIERARCHY_TRY

        auto it = name2Creator_.find(creator->GetName());
        if(it != name2Creator_.end())
            throw AGZ::HierarchyException("creator name repeated: " + creator->GetName());
        name2Creator_[creator->GetName()] = creator;

        AGZ_HIERARCHY_WRAP("in registering resource creator")
    }

    const Creator &operator[](const std::string &name) const
    {
        AGZ_HIERARCHY_TRY

        auto it = name2Creator_.find(name);
        if(it == name2Creator_.end())
            throw AGZ::HierarchyException("invalid ");
        return *it->second;

        AGZ_HIERARCHY_WRAP("in getting creator from creator factory: " + name)
    }

    std::shared_ptr<TResourceCategory> Create(const std::string &creatorName, std::string name, ResourceCreatingContext &ctx) const
    {
        return (*this)[creatorName].Create(std::move(name), ctx);
    }

    auto begin() const { return name2Creator_.begin(); }
    auto end()   const { return name2Creator_.end(); }

    auto begin() { return name2Creator_.begin(); }
    auto end()   { return name2Creator_.end(); }
};
