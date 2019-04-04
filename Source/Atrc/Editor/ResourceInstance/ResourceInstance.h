#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <type_traits>

#include <AGZUtils/Utils/Exception.h>

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

class ExportAsConfigGroup
{
protected:

    static std::string Wrap(const std::string &content)
    {
        return "{" + content + "}";
    }
};

class IResource
{
    const HasName *creator_;

protected:

    static std::filesystem::path RelPath(const std::filesystem::path &dst, const std::filesystem::path &base)
    {
        return relative(absolute(dst), absolute(base));
    }

public:

    explicit IResource(const HasName *creator) noexcept
        : creator_(creator)
    {
        
    }

    virtual ~IResource() = default;

    const std::string &GetType() const noexcept
    {
        return creator_->GetName();
    }
};

class IResourceCreator : public HasName
{
public:

    explicit IResourceCreator(std::string name) noexcept
        : HasName(std::move(name))
    {

    }

    virtual ~IResourceCreator() = default;
};

template<typename TResourceCreatorCategory>
class ResourceFactory
{
public:

    using Resource = typename TResourceCreatorCategory::Resource;
    using Creator = TResourceCreatorCategory;

    void AddCreator(const Creator *creator)
    {
        AGZ_HIERARCHY_TRY

        auto it = name2Creator_.find(creator->GetName());
        if(it != name2Creator_.end())
            throw AGZ::HierarchyException("repeated creator name: " + creator->GetName());
        name2Creator_[creator->GetName()] = creator;

        AGZ_HIERARCHY_WRAP("in registering resource creator")
    }

    const Creator &operator[](const std::string &name) const
    {
        AGZ_HIERARCHY_TRY

        auto it = name2Creator_.find(name);
        if(it == name2Creator_.end())
            throw AGZ::HierarchyException("unknown creator name: " + name);
        return *it->second;

        AGZ_HIERARCHY_WRAP("in getting creator from creator factory")
    }

    auto begin() const { return name2Creator_.begin(); }
    auto end()   const { return name2Creator_.end(); }

    auto begin() { return name2Creator_.begin(); }
    auto end()   { return name2Creator_.end(); }

private:

    static_assert(std::is_base_of_v<IResource, Resource>);
    static_assert(std::is_base_of_v<IResourceCreator, TResourceCreatorCategory>);

    std::map<std::string, const TResourceCreatorCategory*> name2Creator_;
};

class IFilmFilterCreator;
using FilmFilterFactory = ResourceFactory<IFilmFilterCreator>;

class ILightCreator;
using LightFactory = ResourceFactory<ILightCreator>;

class IMaterialCreator;
using MaterialFactory = ResourceFactory<IMaterialCreator>;

class ITextureCreator;
using TextureFactory = ResourceFactory<ITextureCreator>;
