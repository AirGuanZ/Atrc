#pragma once

#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <tuple>

#include <AGZUtils/Utils/Config.h>
#include <AGZUtils/Utils/Misc.h>

class DeserializeContext { };
class ResourceCreateContext;
class ResourceDisplayContext;
class SerializeContext { };

class ResourceInstance : public AGZ::Uncopiable
{
    std::string name_;

public:

    explicit ResourceInstance(std::string name) noexcept
        : name_(std::move(name))
    {

    }

    virtual ~ResourceInstance() = default;

    const char *GetName() const noexcept
    {
        return name_.c_str();
    }

    virtual const char *GetTypeName() const noexcept = 0;

    virtual std::string Serialize(SerializeContext &ctx) const = 0;

    virtual void Deserialize(DeserializeContext &ctx, const AGZ::ConfigNode &node) = 0;

    virtual void Display(ResourceDisplayContext &ctx) = 0;

    virtual bool IsMultiline() const noexcept = 0;
};

template<typename TResource>
class ResourceCreator: public AGZ::Uncopiable
{
public:
    
    virtual ~ResourceCreator() = default;

    virtual std::unique_ptr<TResource> Create(const ResourceCreateContext &ctx, std::string name) = 0;

    virtual const char *GetName() const noexcept = 0;
};

template<typename TBase, typename TCore>
class Core2ResourceInstance : public TBase
{
    TCore core_;

    static_assert(std::is_base_of_v<ResourceInstance, TBase>);

public:

    template<typename...Args>
    explicit Core2ResourceInstance(std::string name, Args&&...args)
        : TBase(std::move(name)), core_(std::forward<Args>(args)...)
    {

    }

    const char *GetTypeName() const noexcept override
    {
        return TCore::GetTypeName();
    }

    std::string Serialize(SerializeContext &ctx) const override
    {
        return core_.Serialize(ctx);
    }

    void Deserialize(DeserializeContext &ctx, const AGZ::ConfigNode &node) override
    {
        core_.Deserialize(ctx, node);
    }

    void Display(ResourceDisplayContext &ctx) override
    {
        core_.Display(ctx);
    }

    bool IsMultiline() const noexcept override
    {
        return TCore::IsMultiline();
    }
};

template<typename TBase, typename TCore>
class Core2ResourceCreator : public ResourceCreator<TBase>
{
    static_assert(std::is_base_of_v<ResourceInstance, TBase>);

public:

    std::unique_ptr<TBase> Create(const ResourceCreateContext &ctx, std::string name) override
    {
        return std::make_unique<Core2ResourceInstance<TBase, TCore>>(std::move(name), ctx);
    }

    const char *GetName() const noexcept override
    {
        return TCore::GetTypeName();
    }
};

template<typename TResource>
class ResourceCreatorManager
{
    std::map<std::string, const ResourceCreator<TResource>*> name2Creator_;

public:

    std::unique_ptr<TResource> Create(const ResourceCreateContext &ctx, std::string typeName, std::string name)
    {
        auto it = name2Creator_.find(typeName);
        AGZ_ASSERT(it != name2Creator_.end());
        return it->second->Create(ctx, std::move(name));
    }

    void AddCreator(const ResourceCreator<TResource> *creator)
    {
        AGZ_ASSERT(creator);
        AGZ_ASSERT(name2Creator_.find(creator->GetName()) == name2Creator_.end());
        name2Creator_[creator->GetName()] = creator;
    }

    const ResourceCreator<TResource> *operator[](const std::string &name) const
    {
        auto it = name2Creator_.find(name);
        return it != name2Creator_, end() ? it->second : nullptr;
    }

    auto begin() { return name2Creator_.begin(); }
    auto end()   { return name2Creator_.end(); }

    auto begin() const { return name2Creator_.begin(); }
    auto end()   const { return name2Creator_.end(); }
};

class CameraInstance : public ResourceInstance
{
public:

    using ResourceInstance::ResourceInstance;
};

class FilmFilterInstance : public ResourceInstance
{
public:

    using ResourceInstance::ResourceInstance;
};

class ResourceCreatorManagerList
{
    template<typename...TBases>
    using TupleOfCreatorManager = std::tuple<ResourceCreatorManager<TBases>...>;

    TupleOfCreatorManager<
        CameraInstance,
        FilmFilterInstance
    > mgrs_;

public:

    template<typename TBase>
    ResourceCreatorManager<TBase> *GetCreatorMgr()
    {
        return &std::get<ResourceCreatorManager<TBase>>(mgrs_);
    }
};

class ResourceCreateContext : public ResourceCreatorManagerList
{
public:

};

class ResourceDisplayContext : public ResourceCreatorManagerList
{
public:

};
