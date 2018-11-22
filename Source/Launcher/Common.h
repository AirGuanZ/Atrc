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

    // ��ʼ��ȫ�ֹ���Ԫ�أ��������������б�����
    void InitializePublicDefinitions(const ConfigGroup &contents, ObjArena<> &arena)
    {
        for(auto it : contents.GetChildren())
            definitions_[it.first] = ObjectManager<T>::GetInstance().Create(it.second->AsGroup(), arena);
    }

    // ע��һ���µ�ObjectCreator
    void AddCreator(const Creator *creator)
    {
        AGZ_ASSERT(creator);
        name2Creator_[creator->GetName()] = creator;
    }

    // ����rightItem��ȡ��һ������object
    // ��node��$name��ʽ��value����ʾ������һ�����ж���
    // ����nodeӦ����group��������Ҫ������Ԫ�����ͼ�����
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
