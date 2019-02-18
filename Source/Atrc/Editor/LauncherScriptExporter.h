#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>
#include <Atrc/Editor/EditorCore.h>
#include <Atrc/Editor/LauncherScriptExportingContext.h>

class LauncherScriptExporter
{
    ResourceManager &rscMgr_;
    LauncherScriptExportingContext &ctx_;

public:

    LauncherScriptExporter(ResourceManager &rscMgr, LauncherScriptExportingContext &ctx);

    std::string Export() const;
};

class LauncherScriptImporter
{
    template<typename TResource>
    const std::string &GetCompletePoolName()
    {
        static const std::string ret = std::string("pool.") + TResource::GetPoolName();
        return ret;
    }

    // 直接根据参数创建一个对象
    // params一定不是引用
    template<typename TResource>
    std::shared_ptr<TResource> CreateResourceFromScratch(ResourceManager &rscMgr, std::string instanceName, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params)
    {
        auto &type = params["type"].AsValue();
        auto instance = rscMgr.GetCreatorManager<TResource>().Create(rscMgr, type, std::move(instanceName));
        instance->Import(rscMgr, root, params);
        return instance;
    }

    // 用名字取得一个池子中的对象
    // 这个对象一定不是引用，可以直接创建
    // 如果池子中已经有了，就直接返回
    // 否则，根据参数来创建并存进池子里
    template<typename TResource>
    std::shared_ptr<TResource> GetResourceInPool(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const std::string &rscName)
    {
        auto &pool = rscMgr.GetPool<TResource>();
        if(auto instance = pool.GetByName(rscName))
            return instance;

        auto &poolName = GetCompletePoolName<TResource>();
        std::string paramPath = poolName + "." + rscName;
        auto newInstance = CreateResourceFromScratch<TResource>(rscMgr, rscName, root, root[paramPath].AsGroup());
        pool.AddInstance(newInstance);

        return newInstance;
    }

    // 导入某种类型的池子中的所有数据
    template<typename TResource>
    void ImportFromPool(const AGZ::ConfigGroup &root, ResourceManager &rscMgr)
    {
        auto &children = root[GetCompletePoolName<TResource>()].AsGroup().GetChildren();
        for(auto &it : children)
            GetResourceInPool<TResource>(rscMgr, root, it.first);
    }

    const AGZ::ConfigGroup &GetFinalNonReferenceParam(const AGZ::ConfigGroup &root, const AGZ::ConfigNode &node)
    {
        if(auto pVal = node.TryAsValue())
            return GetFinalNonReferenceParam(root, root[*pVal]);

        auto &grp = node.AsGroup();
        if(grp["type"].AsValue() == "Reference")
            return GetFinalNonReferenceParam(root, root[grp["name"].AsValue()]);

        return grp;
    }

public:

    // 取得一个resource对象
    // 如果该对象是一个目标为池子中对象的引用，那么向池子索要
    // 否则，如果该对象是一个引用，递归地处理其解引用后的结果
    // 否则，原地用参数创建匿名对象
    template<typename TResource>
    std::shared_ptr<TResource> GetResourceInstance(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigNode &params)
    {
        if(auto pVal = params.TryAsValue()) // 名字必然是引用
        {
            auto &poolName = GetCompletePoolName<TResource>();
            if(AGZ::StartsWith(*pVal, poolName)) // 这是一个针对池子中的对象的引用
                return GetResourceInPool<TResource>(rscMgr, root, pVal->substr(poolName.length()));

            // 针对不在池子中的引用，递归地调用自身
            auto &referencedParams = root[*pVal];
            return GetResourceInstance<TResource>(rscMgr, root, referencedParams);
        }

        auto &grp = params.AsGroup();
        if(grp["type"].AsValue() == "Reference") // 另一种引用类型
        {
            auto &name = grp["name"].AsValue();

            auto &poolName = GetCompletePoolName<TResource>();
            if(AGZ::StartsWith(name, poolName)) // 这是一个针对池子中的对象的引用
                return GetResourceInPool<TResource>(rscMgr, root, name.substr(poolName.length()));

            // 针对不在池子中的引用，递归地调用自身
            auto &referencedParams = root[name];
            return GetResourceInstance<TResource>(rscMgr, root, referencedParams);
        }

        // 直接创建匿名对象
        return CreateResourceFromScratch<TResource>(rscMgr, "", root, grp);
    }

    void Import(const AGZ::ConfigGroup &root, EditorData *data, std::string_view scriptPath);
};
