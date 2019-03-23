#pragma once

#include <tuple>

#include <Atrc/Editor/ResourceInstance/ResourceInstance.h>

class IRangeTexture1;

class ResourceCreatingContext
{
    template<typename...TResourceCategories>
    using TFactoryList = std::tuple<ResourceFactory<TResourceCategories>*...>;
    
    using FactoryList = TFactoryList<
        IRangeTexture1
    >;

    FactoryList factoryList_;

public:

    template<typename...TResourceCategories>
    explicit ResourceCreatingContext(ResourceFactory<TResourceCategories>&...factoryList) noexcept
        : factoryList_{ &factoryList... }
    {
        static_assert(std::is_same_v<TFactoryList<TResourceCategories...>, FactoryList>);
    }

    template<typename TResourceCategory>
    ResourceFactory<TResourceCategory> &GetFactory() noexcept
    {
        return *std::get<ResourceFactory<TResourceCategory>*>(factoryList_);
    }

    template<typename TResourceCategory>
    std::shared_ptr<TResourceCategory> Create(const std::string &creatorName, std::string name)
    {
        return GetFactory<TResourceCategory>().Create(creatorName, std::move(name), *this);
    }
};
