#pragma once

#include <map>
#include <string>
#include <QWidget>

#include <AGZUtils/Utils/Config.h>
#include <AGZUtils/Utils/Exception.h>
#include <Atrc/Atrc/QUtils.h>

class ResourceInstance
{
public:

    virtual ~ResourceInstance() = default;

    virtual std::string Serialize() const = 0;

    virtual void Deserialize(const AGZ::ConfigNode &node) = 0;

    virtual QWidget *GetWidget() = 0;

    virtual bool IsMultiline() const noexcept = 0;
};

template<typename TResourceInstance>
class ResourceInstanceCreator
{
public:

    virtual ~ResourceInstanceCreator() = default;

    virtual const char *GetName() const = 0;

    virtual TResourceInstance *Create() const = 0;
};

template<typename TResourceInstance>
class ResourceInstanceCreatorManager
{
    std::map<std::string, const ResourceInstanceCreator<TResourceInstance>*> name2Creator_;

public:

    void Add(const ResourceInstanceCreator<TResourceInstance> *creator)
    {
        AGZ_ASSERT(creator);
        name2Creator_[creator->GetName()] = creator;
    }

    const ResourceInstanceCreator<TResourceInstance> *operator[](const std::string &name) const
    {
        AGZ_HIERARCHY_TRY

        auto it = name2Creator_.find(name);
        if(it == name2Creator_.end())
            throw AGZ::HierarchyException("unknown creator name: " + name);
        return it->second;

        AGZ_HIERARCHY_WRAP("in indexing resource creator by name")
    }

    auto begin() { return name2Creator_.begin(); }
    auto end()   { return name2Creator_.end(); }

    auto begin() const { return name2Creator_.begin(); }
    auto end()   const { return name2Creator_.end(); }
};

template<typename TResourceInstanceBase, typename TResourceInstance>
class ResourceInstance2Creator : public ResourceInstanceCreator<TResourceInstanceBase>
{
    static_assert(std::is_base_of_v<ResourceInstance, TResourceInstanceBase>);
    static_assert(std::is_base_of_v<TResourceInstanceBase, TResourceInstance>);

public:

    const char *GetName() const override
    {
        return TResourceInstance::GetTypeName();
    }

    TResourceInstanceBase *Create() const override
    {
        return new TResourceInstance();
    }
};

template<typename TResourceInstanceBase, typename TResourceInstanceWidgetCore>
class WidgetCore2ResourceInstance : public TResourceInstanceBase
{
    UniqueQPtr<TResourceInstanceWidgetCore> core_;

    static_assert(std::is_base_of_v<ResourceInstance, TResourceInstanceBase>);
    static_assert(std::is_base_of_v<QWidget, TResourceInstanceWidgetCore>);

public:

    static const char *GetTypeName()
    {
        return TResourceInstanceWidgetCore::GetTypeName();
    }

    WidgetCore2ResourceInstance()
        : core_(MakeUniqueQ<TResourceInstanceWidgetCore>())
    {

    }

    std::string Serialize() const override
    {
        return core_->Serialize();
    }

    void Deserialize(const AGZ::ConfigNode &node) override
    {
        core_->Deserialize(node);
    }

    QWidget *GetWidget() override
    {
        return core_.get();
    }

    bool IsMultiline() const noexcept override
    {
        return core_->IsMultiline();
    }
};
