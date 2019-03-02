#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <AGZUtils/Utils/Config.h>
#include <AGZUtils/Utils/Misc.h>
#include <Atrc/Lib/Core/Common.h>

/*
    ResourceData是资源对象的核心，从它衍生出来的使用包括：
    1. Lib使用它创建的可渲染对象进行渲染
    2. Launcher使用它将脚本文件转换为一组可渲染对象
    3. Editor使用它导出用于Launcher进行渲染的脚本
    4. Editor使用它保存和加载资源对象
    5. Editor提供展示和修改它的界面，ResourceData -> ResourceDataWidget的映射需要在ResourceData之外的地方维护
*/

namespace Atrc
{

DEFINE_ATRC_EXCEPTION(ResourceDataException);

template<typename TResource>
class ResourceData : public AGZ::Uncopiable
{
public:

    using ResourceType = TResource;

    virtual ~ResourceData() = default;

    virtual std::string Serialize() const = 0;

    virtual void Deserialize(const AGZ::ConfigGroup &param) = 0;

    virtual TResource *CreateResource() const = 0;
};

template<typename TResource>
class ResourceDataCreator : public AGZ::Uncopiable
{
public:

    virtual ~ResourceDataCreator() = default;

    virtual const std::string &GetName() const = 0;

    virtual ResourceData<TResource> *CreateResourceData(Arena &arena) const = 0;

    virtual std::shared_ptr<ResourceData<TResource>> CreateResourceData() const = 0;
};

template<typename TResourceData>
class ResourceData2Creator : public ResourceDataCreator<typename TResourceData::ResourceType>
{
public:

    const std::string &GetName() const override
    {
        return TResourceData::GetTypeName();
    }

    ResourceData<typename TResourceData::ResourceType> *CreateResourceData(Arena &arena) const override
    {
        return arena.Create<TResourceData>();
    }

    std::shared_ptr<ResourceData<typename TResourceData::ResourceType>> CreateResourceData() const override
    {
        return std::make_shared<typename TResourceData::ResourceType>();
    }
};

template<typename TResource>
class ResourceDataCreatorManager
{
    std::unordered_map<std::string, const ResourceDataCreator<TResource>*> name2Creator_;
    std::vector<const ResourceDataCreator<TResource>*> sortedCreators_;

public:

    void AddCreator(const ResourceDataCreator<TResource> *creator)
    {
        auto &name = creator->GetName();
        auto it = name2Creator_.find(name);
        if(it != name2Creator_.end())
            throw ResourceDataException("resource data creator name repeated");
        name2Creator_.insert(std::make_pair(name, creator));
        sortedCreators_.push_back(creator);
        std::sort(std::begin(sortedCreators_), std::end(sortedCreators_),
                  [](auto L, auto R) { return L->GetName() < R->GetName(); });
    }

    const ResourceDataCreator<TResource> *operator[](const std::string &name) const
    {
        auto it = name2Creator_.find(name);
        if(it == name2Creator_.end())
            throw ResourceDataException("resource data creator not found: " + name);
        return it->second;
    }

    auto begin() { return sortedCreators_.begin(); }
    auto end()   { return sortedCreators_.end(); }

    auto begin() const { return sortedCreators_.begin(); }
    auto end()   const { return sortedCreators_.end(); }

    template<typename TOutputIterator>
    void GetAllNames(TOutputIterator outputIterator) const
    {
        for(auto c : sortedCreators_)
        {
            *outputIterator = c->GetName();
            ++outputIterator;
        }
    }
};

} // namespace Atrc
