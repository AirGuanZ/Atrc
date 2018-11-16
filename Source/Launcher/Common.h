#pragma once

#include <unordered_map>

#include <Atrc/Atrc.h>

using AGZ::Option;
using AGZ::Some;
using AGZ::None;

using AGZ::ObjArena;
using AGZ::Config;
using AGZ::ConfigGroup;
using AGZ::ConfigNode;

using AGZ::Str8;

class SceneInitializationException : public std::runtime_error
{
public:

    explicit SceneInitializationException(const Str8 &msg)
        : runtime_error(msg.ToStdString())
    {
        
    }
};

template<typename T>
class ObjectCreator
{
public:

    virtual ~ObjectCreator() = default;

    virtual Str8 GetName() const = 0;

    virtual T *Create(const ConfigGroup &params, ObjArena<> &arena) const = 0;
};

template<typename T>
class ObjectManager : public AGZ::Singleton<ObjectManager<T>>
{
    std::unordered_map<Str8, T*> definitions_;

public:

    using Creator = ObjectCreator<T>;

    // 初始化全局公有元素，可在其他定义中被引用
    void InitializePublicDefinitions(const ConfigGroup &contents, ObjArena<> &arena)
    {
        for(auto it : contents.GetChildren())
            definitions_[it.first] = ObjectManager<T>::GetInstance().Create(it.second->AsGroup(), arena);
    }

    // 注册一个新的ObjectCreator
    void AddCreator(const Creator *creator)
    {
        AGZ_ASSERT(creator);
        name2Creator_[creator->GetName()] = creator;
    }

    // 给定rightItem，取得一个场景object
    // 若node是$name形式的value，表示引用了一个公有定义
    // 否则。node应该是group，包含了要创建的元素类型及参数
    T *GetSceneObject(const ConfigNode &node, ObjArena<> &arena)
    {
        if(node.IsValue())
        {
            auto &n = node.AsValue();
            if(n.StartsWith("$"))
                return GetPublicDefinition(n.Slice(1));
        }
        return ObjectManager<T>::GetInstance().Create(node.AsGroup(), arena);
    }

private:

    T *Create(const ConfigGroup &params, ObjArena<> &arena) const
    {
        auto it = name2Creator_.find(params["type"].AsValue());
        if(it == name2Creator_.end())
            return nullptr;
        return it->second->Create(params, arena);
    }

    T *GetPublicDefinition(const Str8 &name) const
    {
        auto it = definitions_.find(name);
        return it != definitions_.end() ? it->second : nullptr;
    }

    std::unordered_map<Str8, const Creator*> name2Creator_;
};

template<typename T>
T *GetSceneObject(const ConfigNode &node, ObjArena<> &arena)
{
    return ObjectManager<T>::GetInstance().GetSceneObject(node, arena);
}
