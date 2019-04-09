#pragma once

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

    ResourceDnDSlots DnDSlots;

    template<typename TResource>
    auto &Get() noexcept
    {
        return std::get<Rsc2Idx<TResource, 0>()>(facs_);
    }

    template<typename TResource, typename...LoadArgs>
    auto CreateAndLoad(const AGZ::ConfigNode &params, LoadArgs&&...loadArgs)
    {
        auto &group = params.AsGroup();
        auto ret = Get<TResource>()[group["type"].AsValue()].Create();
        ret->Load(group, std::forward<LoadArgs>(loadArgs)...);
        return ret;
    }
};

extern ResourceFactoryList RF;

void RegisterBuiltinResourceCreators();

}; // namespace Atrc::Editor
