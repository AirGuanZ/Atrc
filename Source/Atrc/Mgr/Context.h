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

    virtual const T *Create(const ConfigGroup &group, Context &context, Arena &arena) = 0;
};

template<typename T>
class Factory
{
    std::unordered_map<Str8, Creator<T>*> creators_;
    std::unordered_map<Str8, const T*> pubObjs_;

public:

    // 若definition.IsGroup，则通过其definition["type"]在creators中查找合适的creator进行创建
    // 若definition.IsValue，则在pubObjs中查找它，找不到的话就在root中进行find，结果cache在pubObjs中
    const T *Create(const ConfigNode &definition, Context &context, Arena &arena);

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
        Renderer,
        Sampler,
        Texture
    > factories_;

    const ConfigGroup &root_;
    Arena &arena_;

public:

    Context(const ConfigGroup &root, Arena &arena);

    const ConfigGroup &Root() const noexcept { return root_; }

    template<typename T>
    void AddCreator(const Creator<T> *creator);

    template<typename T>
    const T *Create(const ConfigNode &definition);
};

// ================================= Implementation

template<typename T>
const T *Factory<T>::Create(const ConfigNode &definition, Context &context, Arena &arena)
{
    if(auto val = definition.TryAsValue())
    {
        auto it = pubObjs_.find(*val);
        if(it != pubObjs_.end())
            return it->second;

        auto node = context.Root().Find(*val);
        if(!node)
            return nullptr;
        
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
            return nullptr;

        return it->second->Create(*grp, context, arena);
    }

    return nullptr;
}

template<typename T>
void Factory<T>::AddCreator(const Creator<T> *creator)
{
    AGZ_ASSERT(creator);
    creators_[creator->GetTypeName()] = creator;
}

inline Context::Context(const ConfigGroup &root, Arena &arena)
    : root_(root), arena_(arena)
{
    
}

template<typename T>
void Context::AddCreator(const Creator<T> *creator)
{
    std::get<Factory<T>>(factories_).AddCreator(creator);
}

template<typename T>
const T *Context::Create(const ConfigNode &definition)
{
    ATRC_MGR_TRY
    {
        const T *ret = std::get<Factory<T>>(factories_).Create(definition, *this, arena_);
        if(!ret)
            throw MgrErr("Context::Create: factory returns nullptr");
        return ret;
    }
    ATRC_MGR_CATCH_AND_RETHROW(
        "In creating : " + Str8(typeid(T).name()) + " with " + definition.ToString())
}

} // namespace Atrc::Mgr
