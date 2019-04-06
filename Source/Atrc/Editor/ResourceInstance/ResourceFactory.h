#pragma once

#include <Atrc/Editor/ResourceInstance/ResourceInstance.h>

#include <Atrc/Editor/FilmFilter/FilmFilter.h>
#include <Atrc/Editor/Light/Light.h>
#include <Atrc/Editor/Material/Material.h>
#include <Atrc/Editor/Sampler/Sampler.h>
#include <Atrc/Editor/Texture/Texture.h>

class ResourceFactoryList
{
    using Tuple = std::tuple<
        FilmFilterFactory,
        LightFactory,
        MaterialFactory,
        SamplerFactory,
        TextureFactory
    >;

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
};

extern ResourceFactoryList RF;

void RegisterBuiltinResourceCreators();
