#pragma once

#include <AGZUtils/Utils/Exception.h>
#include <Atrc/Editor/ResourceInstance/ResourceInstance.h>

#include <Atrc/Editor/FilmFilter/FilmFilter.h>
#include <Atrc/Editor/Fresnel/Fresnel.h>
#include <Atrc/Editor/Light/Light.h>
#include <Atrc/Editor/Material/Material.h>
#include <Atrc/Editor/Sampler/Sampler.h>
#include <Atrc/Editor/Texture/Texture.h>

namespace Atrc::Editor
{

class ResourceDnDSlots
{
    template<typename...TResources>
    using TSlots = std::tuple<std::shared_ptr<TResources>...>;
    using Slots = TSlots<TRESOURCE_LIST>;

    Slots slots_;

public:

    template<typename TResource>
    void Set(std::shared_ptr<TResource> p)
    {
        std::get<std::shared_ptr<TResource>>(slots_) = std::move(p);
    }

    template<typename TResource>
    std::shared_ptr<TResource> Get() const
    {
        return std::get<std::shared_ptr<TResource>>(slots_);
    }
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
            throw std::runtime_error("repeated creator name: " + creator->GetName());
        name2Creator_[creator->GetName()] = creator;

        AGZ_HIERARCHY_WRAP("in registering resource creator")
    }

    const Creator &operator[](const std::string &name) const
    {
        AGZ_HIERARCHY_TRY

            auto it = name2Creator_.find(name);
        if(it == name2Creator_.end())
            throw std::runtime_error("unknown creator name: " + name);
        return *it->second;

        AGZ_HIERARCHY_WRAP("in getting creator from creator factory")
    }

    auto begin() const { return name2Creator_.begin(); }
    auto end()   const { return name2Creator_.end(); }

    auto begin() { return name2Creator_.begin(); }
    auto end() { return name2Creator_.end(); }

private:

    static_assert(std::is_base_of_v<IResource, Resource>);
    static_assert(std::is_base_of_v<IResourceCreator, TResourceCreatorCategory>);

    std::map<std::string, const TResourceCreatorCategory*> name2Creator_;
};

class ResourceFactoryList
{
    using Tuple = std::tuple<TRESOURCE_FACTORY_LIST>;

    Tuple facs_;

    template<typename TResource, int I>
    static constexpr int Rsc2Idx()
    {
        return Rsc2IdxAux<TResource, I, std::is_same_v<TResource, typename std::tuple_element_t<I, Tuple>::Resource>>::Value();
    }
    template<typename TResource, int I, bool Ok> struct Rsc2IdxAux { static constexpr int Value() { return Rsc2Idx<TResource, I + 1>(); } };
    template<typename TResource, int I> struct Rsc2IdxAux<TResource, I, true> { static constexpr int Value() { return I; } };

public:

    template<typename TResource>
    auto &Get() noexcept
    {
        return std::get<Rsc2Idx<TResource, 0>()>(facs_);
    }

    template<typename TResource>
    std::shared_ptr<TResource> Create(const std::string &type) const
    {
        return std::get<Rsc2Idx<TResource, 0>()>(facs_)[type].Create();
    }

    template<typename TResource, typename...LoadArgs>
    auto CreateAndLoad(const AGZ::ConfigNode &params, LoadArgs&&...loadArgs) const
    {
        auto &group = params.AsGroup();
        auto ret = std::get<Rsc2Idx<TResource, 0>()>(facs_)[group["type"].AsValue()].Create();
        ret->Load(group, std::forward<LoadArgs>(loadArgs)...);
        return ret;
    }
};

extern ResourceFactoryList RF;

void RegisterBuiltinResourceCreators();

}; // namespace Atrc::Editor
