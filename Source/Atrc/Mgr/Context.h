#pragma once

#include <typeinfo>
#include <tuple>
#include <unordered_map>

#include <Utils/Misc.h>
#include <Utils/Config.h>

#include <Atrc/Mgr/Common.h>

namespace Atrc::Mgr
{

class Context;

template<typename T>
class Creator
{
public:

    virtual ~Creator() = default;

    virtual Str8 GetTypeName() const = 0;

    virtual T *Create(const ConfigGroup &group, Context &context, Arena &arena) const = 0;
};

template<typename T>
class Factory
{
    std::unordered_map<Str8, const Creator<T>*> creators_;
    std::unordered_map<Str8, T*> pubObjs_;

public:

    // 若definition.IsGroup，则通过其definition["type"]在creators中查找合适的creator进行创建
    // 若definition.IsValue，则在pubObjs中查找它，找不到的话就在root中进行find，结果cache在pubObjs中
    T *Create(const ConfigNode &definition, Context &context, Arena &arena);

    // 添加对creator的支持，不持有其所有权
    void AddCreator(const Creator<T> *creator);
};

class Context
{
    template<typename...Ts>
    using FactoryList = std::tuple<Factory<Ts>...>;

    FactoryList<
        Camera,
        Entity,
        FilmFilter,
        Geometry,
        Light,
        Material,
        PathTracingIntegrator,
        PostProcessor,
        Renderer,
        Reporter,
        Sampler,
        Texture
    > factories_;

    const ConfigGroup &root_;
    Arena arena_;

public:

    explicit Context(const ConfigGroup &root);

    const ConfigGroup &Root() const noexcept { return root_; }

    template<typename T>
    void AddCreator(const Creator<T> *creator);

    template<typename T>
    T *Create(const ConfigNode &definition);

    template<typename T, typename...Args>
    T *CreateWithInteriorArena(Args&&...args);
};

// ================================= Implementation

template<typename T>
T *Factory<T>::Create(const ConfigNode &definition, Context &context, Arena &arena)
{
    if(auto val = definition.TryAsValue())
    {
        auto it = pubObjs_.find(*val);
        if(it != pubObjs_.end())
            return it->second;

        auto node = context.Root().Find(*val);
        if(!node)
            throw MgrErr("Object definition not found: " + *val);
        
        // 先把pubObjs_[*val]设为nullptr，避免对同一类型同一名字的循环引用，这样可以消除循环引用产生的死循环
        pubObjs_[*val] = nullptr;
        auto ret = Create(*node, context, arena);
        pubObjs_[*val] = ret;
        return ret;
    }

    if(auto grp = definition.TryAsGroup())
    {
        auto it = creators_.find((*grp)["type"].AsValue());
        if(it == creators_.end())
            throw MgrErr("Unknown object type");
        return it->second->Create(*grp, context, arena);
    }

    throw MgrErr("Invalid object definition form");
}

template<typename T>
void Factory<T>::AddCreator(const Creator<T> *creator)
{
    AGZ_ASSERT(creator);
    creators_[creator->GetTypeName()] = creator;
}

inline Context::Context(const ConfigGroup &root)
    : root_(root)
{
    
}

template<typename T>
void Context::AddCreator(const Creator<T> *creator)
{
    std::get<Factory<T>>(factories_).AddCreator(creator);
}

template<typename T>
T *Context::Create(const ConfigNode &definition)
{
    ATRC_MGR_TRY
    {
        T *ret = std::get<Factory<T>>(factories_).Create(definition, *this, arena_);
        if(!ret)
            throw MgrErr("Context::Create: factory returns nullptr");
        return ret;
    }
    ATRC_MGR_CATCH_AND_RETHROW(
        "In creating : " + Str8(typeid(T).name()) + " with " + definition.ToString())
}

template<typename T, typename...Args>
T *Context::CreateWithInteriorArena(Args&&...args)
{
    return arena_.Create<T>(std::forward<Args>(args)...);
}

} // namespace Atrc::Mgr
