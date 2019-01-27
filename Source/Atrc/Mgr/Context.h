#pragma once

#include <map>
#include <typeinfo>
#include <tuple>
#include <unordered_map>

#include <AGZUtils/Utils/FileSys.h>
#include <AGZUtils/Utils/Misc.h>
#include <AGZUtils/Utils/Config.h>

#include <Atrc/Mgr/Common.h>

namespace Atrc
{
    class Fresnel;
    class PathTracingIntegrator;
} // namespace Atrc

namespace Atrc::Mgr
{

class Context;

using Name2Geometry = std::map<std::string, Geometry*>;

template<typename T>
class Creator
{
public:

    virtual ~Creator() = default;

    virtual std::string GetTypeName() const = 0;

    virtual T *Create(const ConfigGroup &group, Context &context, Arena &arena) const = 0;
};

template<typename T>
class Factory
{
    std::unordered_map<std::string, const Creator<T>*> creators_;
    std::unordered_map<std::string, T*> pubObjs_;

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
        Fresnel,
        Geometry,
        Light,
        Material,
        Medium,
        Name2Geometry,
        PathTracingIntegrator,
        PostProcessor,
        Renderer,
        Reporter,
        Sampler,
        Texture
    > factories_;

    const ConfigGroup &root_;
    Arena arena_;

    std::filesystem::path workspace_;
    std::filesystem::path configPath_;

public:

    explicit Context(const ConfigGroup &root, std::string_view configFilename);

    const ConfigGroup &Root() const noexcept { return root_; }

    template<typename T>
    void AddCreator(const Creator<T> *creator);

    template<typename T>
    T *Create(const ConfigNode &definition);

    template<typename T, typename...Args>
    T *CreateWithInteriorArena(Args&&...args);

    std::string GetPathInWorkspace(std::string_view subFilename) const;
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
        "In creating : " + std::string(typeid(T).name()) + " with " + definition.ToString())
}

template<typename T, typename...Args>
T *Context::CreateWithInteriorArena(Args&&...args)
{
    return arena_.Create<T>(std::forward<Args>(args)...);
}

} // namespace Atrc::Mgr
