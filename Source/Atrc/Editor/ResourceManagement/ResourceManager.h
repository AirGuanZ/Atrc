#pragma once

#include <algorithm>
#include <limits>
#include <map>
#include <string>

#include <AGZUtils/Utils/Misc.h>

#include <Atrc/Editor/SceneExportingContext.h>
#include <Atrc/Editor/GL.h>
#include <AGZUtils/Config/Config.h>

/**
 * Instance管理体系
 * 
 * ObjectManager持有一系列InstanceCreatorManager和InstancePool
 * InstanceCreatorManager保存一组InstanceCreator的指针
 * InstancePool持有一组Instance
 */

class SceneExportingContext;
class ResourceManager;

class IResource : public AGZ::Uncopiable
{
    std::string typeName_;
    std::string instanceName_;
    bool isInPool_;

protected:

    virtual void Export(const ResourceManager &rscMgr, SceneExportingContext &ctx) const = 0;

public:

    struct ImportContext
    {
        std::filesystem::path scriptPath;
        std::filesystem::path workspacePath;

        std::string GetFilename(std::string_view filename) const
        {
            if(AGZ::StartsWith(filename, "$"))
                return std::string(filename.substr(1));
            if(AGZ::StartsWith(filename, "@"))
                return (scriptPath / filename.substr(1)).string();
            auto p = std::filesystem::path(filename);
            if(p.is_absolute())
                return p.string();
            return (workspacePath / p).string();
        }
    };

    IResource(std::string typeName, std::string name) noexcept
        : typeName_(std::move(typeName)), instanceName_(std::move(name)), isInPool_(false)
    {

    }

    template<typename TSubResource>
    static void ExportSubResource(
        std::string left, const ResourceManager &rscMgr, SceneExportingContext &ctx,
        const TSubResource &subrsc)
    {
        ctx.AddLine(std::move(left) + " = {");
        ctx.IncIndent();
        subrsc.Export(rscMgr, ctx);
        ctx.DecIndent();
        ctx.AddLine("};");
    }

    template<typename TSubResource>
    static void ExportSubResourceAsReference(
        std::string left, const ResourceManager &rscMgr, SceneExportingContext &ctx,
        const TSubResource &subrsc)
    {
        ctx.AddLine(std::move(left) + " = {");
        ctx.IncIndent();
        subrsc.template ExportAsReference<TSubResource>(rscMgr, ctx);
        ctx.DecIndent();
        ctx.AddLine("};");
    }

    virtual ~IResource() = default;

    const std::string &GetName() const noexcept { return instanceName_; }

    const std::string &GetTypeName() const noexcept { return typeName_; }

    bool IsInPool() const noexcept { return isInPool_; }

    void PutIntoPool() noexcept { isInPool_ = true; }

    void DropFromPool() noexcept { isInPool_ = false; }

    virtual void Display(ResourceManager &rscMgr) = 0;

    template<typename TResource>
    void ExportAsReference(const ResourceManager &rscMgr, SceneExportingContext &ctx) const
    {
        AGZ_ASSERT(typeid(*this) == typeid(TResource));
        if(isInPool_)
        {
            ctx.AddLine("type = Reference;");
            ctx.AddLine("name = \"pool.", TResource::GetPoolName(), ".", GetName(), "\";");
        }
        else
            Export(rscMgr, ctx);
    }

    virtual void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) { }
};

template<typename TResource>
class TResourceCreator : public AGZ::Uncopiable
{
    std::string creatorName_;

public:

    explicit TResourceCreator(std::string name) noexcept
        : creatorName_(std::move(name))
    {
        
    }

    virtual ~TResourceCreator() = default;

    virtual std::shared_ptr<TResource> Create(ResourceManager &rscMgr, std::string name) const = 0;

    const std::string &GetName() const noexcept
    {
        return creatorName_;
    }
};

template<typename TResource>
class TResourceCreatorManager
{
    std::map<std::string, const TResourceCreator<TResource>*> name2Creator_;

public:

    std::shared_ptr<TResource> Create(ResourceManager &rscMgr, const std::string &creatorName, std::string instanceName) const
    {
        auto it = name2Creator_.find(creatorName);
        AGZ_ASSERT(it != name2Creator_.end());
        return it->second->Create(rscMgr, std::move(instanceName));
    }

    void AddCreator(const TResourceCreator<TResource> *creator)
    {
        AGZ_ASSERT(creator);
        name2Creator_[creator->GetName()] = creator;
    }

    auto begin()       { return name2Creator_.begin(); }
    auto end()         { return name2Creator_.end();   }
    auto begin() const { return name2Creator_.begin(); }
    auto end()   const { return name2Creator_.end();   }
};

template<typename TResource, bool THasPool>
struct TResourceRegister
{
    using ResourceType                   = TResource;
    static constexpr bool HasPool        = THasPool;
};

template<typename TResource>
class TResourceCreatorSelector
{
    const TResourceCreator<TResource> *selectedCreator_;
    TResourceCreatorManager<TResource> &creatorMgr_;

public:

    explicit TResourceCreatorSelector(TResourceCreatorManager<TResource> &mgr) noexcept
        : selectedCreator_(nullptr), creatorMgr_(mgr)
    {
        
    }

    const TResourceCreator<TResource> *GetSelectedCreator() noexcept
    {
        return selectedCreator_;
    }

    void Display(const char *label)
    {
        const char *previewedValue = selectedCreator_ ? selectedCreator_->GetName().c_str() : nullptr;
        if(!ImGui::BeginCombo(label, previewedValue))
            return;
        AGZ::ScopeGuard comboExitGuard([] { ImGui::EndCombo(); });

        for(auto &p : creatorMgr_)
        {
            ImGui::PushID(p.first.c_str());
            bool isSelected = selectedCreator_ == p.second;
            if(ImGui::Selectable(p.first.c_str(), isSelected))
                selectedCreator_ = p.second;
            if(isSelected)
                ImGui::SetItemDefaultFocus();
            ImGui::PopID();
        }
    }
};

template<typename TResource>
class TResourcePool
{
    static constexpr size_t INDEX_NONE = (std::numeric_limits<size_t>::max)();

    std::shared_ptr<TResource> selectedInstance_;
    std::vector<std::shared_ptr<TResource>> instances_;

    TResourceCreatorSelector<TResource> creatorSelector_;
    TResourceCreatorManager<TResource> &creatorMgr_;

    bool sortInstanceByName_;

    void SortInstanceByName()
    {
        std::sort(std::begin(instances_), std::end(instances_), [](auto &L, auto &R)
        {
            auto lL = AGZ::ToLower(L->GetName()), lR = AGZ::ToLower(R->GetName());
            return  lL < lR || (lL == lR && L->GetName() < R->GetName());
        });
    }

public:

    explicit TResourcePool(TResourceCreatorManager<TResource> &creatorMgr)
        : creatorSelector_(creatorMgr), creatorMgr_(creatorMgr), sortInstanceByName_(false)
    {
        
    }

    void Clear()
    {
        selectedInstance_ = nullptr;
        instances_.clear();
    }

    void AddInstance(std::shared_ptr<TResource> instance)
    {
        instance->PutIntoPool();
        instances_.push_back(std::move(instance));
        if(sortInstanceByName_)
            SortInstanceByName();
    }

    std::shared_ptr<TResource> GetSelectedInstance()
    {
        return selectedInstance_;
    }

    void SetSelectedInstance(std::shared_ptr<TResource> instance)
    {
        AGZ_ASSERT(std::find(std::begin(instances_), std::end(instances_), instance) != std::end(instances_));
        selectedInstance_ = instance;
    }

    bool Find(std::shared_ptr<TResource> rsc) const
    {
        return std::find(std::begin(instances_), std::end(instances_), rsc) != std::end(instances_);
    }

    std::shared_ptr<TResource> GetByName(const std::string &name)
    {
        auto it = std::find_if(std::begin(instances_), std::end(instances_), [&](auto &L) { return L->GetName() == name; });
        return it != std::end(instances_) ? *it : std::shared_ptr<TResource>();
    }

    void Display(ResourceManager &rscMgr)
    {
        creatorSelector_.Display("type");

        static char nameBuf[256] = "";
        ImGui::InputText("name", nameBuf, 256);
        
        if(ImGui::Button("new") && creatorSelector_.GetSelectedCreator())
        {
            auto it = std::find_if(std::begin(instances_), std::end(instances_), [&](auto &c) { return c->GetName() == nameBuf; });
            if(it == instances_.end() && nameBuf[0])
            {
                auto newInstance = creatorSelector_.GetSelectedCreator()->Create(rscMgr, nameBuf);
                AddInstance(newInstance);
                nameBuf[0] = '\0';
            }
        }

        ImGui::SameLine();

        if(ImGui::Button("delete") && selectedInstance_)
        {
            auto it = std::find(std::begin(instances_), std::end(instances_), selectedInstance_);
            AGZ_ASSERT(it != instances_.end());

            size_t idx = it - instances_.begin();
            instances_.erase(it);
            selectedInstance_->DropFromPool();
            selectedInstance_ = nullptr;

            if(idx >= instances_.size())
            {
                if(!instances_.empty())
                    selectedInstance_ = instances_[idx - 1];
            }
            else
                selectedInstance_ = instances_[idx];
        }

        ImGui::SameLine();

        if(ImGui::Checkbox("sort by name", &sortInstanceByName_) && sortInstanceByName_)
            SortInstanceByName();

        ImGui::BeginChild("elems", ImVec2(0, 150), true);

        for(size_t i = 0; i < instances_.size(); ++i)
        {
            ImGui::PushID(static_cast<int>(i));
            auto instance = instances_[i];
            bool isSelected = instance == selectedInstance_;
            std::string name = "[" + instance->GetTypeName() + "] " + instance->GetName();
            if(ImGui::Selectable(name.c_str(), isSelected))
            {
                if(isSelected)
                    selectedInstance_ = nullptr;
                else
                    selectedInstance_ = instance;
            }
            ImGui::PopID();
        }

        ImGui::EndChild();
    }

    auto begin() { return instances_.begin(); }
    auto end() { return instances_.end(); }
};

template<typename...TResources>
class TResourceManager
{
    std::tuple<TResourceCreatorManager<TResources>...> creatorMgrList_;

    template<typename TResource>
    using Pool_t = TResourcePool<TResource>;
    std::tuple<Pool_t<TResources>...> poolList_;

    template<typename TResource>
    TResourceCreatorManager<TResource> &_creatorMgr() noexcept
    {
        return std::get<TResourceCreatorManager<TResource>>(creatorMgrList_);
    }

    template<typename TResource>
    const TResourceCreatorManager<TResource> &_creatorMgr() const noexcept
    {
        return std::get<TResourceCreatorManager<TResource>>(creatorMgrList_);
    }

    template<typename TResource>
    TResourcePool<TResource> &_pool() noexcept
    {
        return std::get<TResourcePool<TResource>>(poolList_);
    }

public:

    TResourceManager()
        : poolList_(_creatorMgr<TResources>()...)
    {
        
    }

    virtual ~TResourceManager() = default;

    template<typename TResource>
    void AddCreator(const TResourceCreator<TResource> *creator)
    {
        _creatorMgr<TResource>().AddCreator(std::move(creator));
    }

    template<typename TResource>
    std::shared_ptr<TResource> Create(const std::string &creatorName, std::string instanceName)
    {
        return _creatorMgr<TResource>().Create(
            AsRscMgr(), creatorName, std::move(instanceName));
    }

    template<typename TResource>
    TResourcePool<TResource> &GetPool() noexcept
    {
        return _pool<TResource>();
    }

    template<typename TResource>
    TResourceCreatorManager<TResource> &GetCreatorManager() noexcept
    {
        return _creatorMgr<TResource>();
    }

    template<typename TResource>
    bool FindInPool(std::shared_ptr<TResource> rsc) const
    {
        return _pool<TResource>().Find(rsc);
    }

    virtual ResourceManager &AsRscMgr() = 0;
};

class CameraInstance : public IResource
{
public:
    
    using IResource::IResource;

    static const char *GetPoolName() { return "camera"; }

    struct ProjData
    {
        float viewportWidth  = 1;
        float viewportHeight = 1;
        Mat4f projMatrix;
    };

    virtual ProjData GetProjData(float dstAspectRatio) const = 0;
};
using CameraCreator = TResourceCreator<CameraInstance>;

class EntityInstance : public IResource
{
public:

    using IResource::IResource;

    static const char *GetPoolName() { return "entity"; }

    virtual void Render(const Mat4f &projViewMat, const Vec3f &eyePos) = 0;

    void Display(ResourceManager&) final override { }

    virtual void DisplayEx(ResourceManager &rscMgr, const Mat4f &proj, const Mat4f &view, bool renderController) = 0;
};
using EntityCreator = TResourceCreator<EntityInstance>;

class FilmFilterInstance : public IResource { public: using IResource::IResource; static const char *GetPoolName() { return "filmFilter"; } };
using FilmFilterCreator = TResourceCreator<FilmFilterInstance>;

class FresnelInstance : public IResource { public: using IResource::IResource; static const char *GetPoolName() { return "fresnel"; } };
using FresnelCreator = TResourceCreator<FresnelInstance>;

class GeometryInstance : public IResource
{
public:

    struct Vertex
    {
        Vec3f pos;
        Vec3f nor;
    };
    
    using IResource::IResource;

    static const char *GetPoolName() { return "geometry"; }

    virtual std::shared_ptr<const GL::VertexBuffer<Vertex>> GetVertexBuffer() const = 0;

    virtual std::vector<std::string> GetSubmeshNames() const = 0;

    void Display(ResourceManager &) final override { }

    virtual void DisplayEditing(const Mat4f &world, const Mat4f &proj, const Mat4f &view, bool renderController) = 0;
};
using GeometryCreator = TResourceCreator<GeometryInstance>;

class LightInstance : public IResource
{
public:

    using IResource::IResource;
    
    static const char *GetPoolName() { return "light"; }

    virtual void Render(const Mat4f &projViewMat, const Vec3f &eyePos) { }
};
using LightCreator = TResourceCreator<LightInstance>;

class MaterialInstance : public IResource { public: using IResource::IResource; static const char *GetPoolName() { return "material"; } };
using MaterialCreator = TResourceCreator<MaterialInstance>;

class PathTracingIntegratorInstance : public IResource { public: using IResource::IResource; static const char *GetPoolName() { return "pathTracingIntegrator"; } };
using PathTracingIntegratorCreator = TResourceCreator<PathTracingIntegratorInstance>;

class RendererInstance : public IResource { public: using IResource::IResource; static const char *GetPoolName() { return "renderer"; } };
using RendererCreator = TResourceCreator<RendererInstance>;

class SamplerInstance : public IResource { public: using IResource::IResource; static const char *GetPoolName() { return "sampler"; } };
using SamplerCreator = TResourceCreator<SamplerInstance>;

class TextureInstance : public IResource { public: using IResource::IResource; static const char *GetPoolName() { return "texture"; } };
using TextureCreator = TResourceCreator<TextureInstance>;

class ResourceManager : public TResourceManager<
    CameraInstance,
    EntityInstance,
    FilmFilterInstance,
    FresnelInstance,
    GeometryInstance,
    LightInstance,
    MaterialInstance,
    PathTracingIntegratorInstance,
    RendererInstance,
    SamplerInstance,
    TextureInstance>
{
public:

    using TResourceManager::TResourceManager;

    ResourceManager &AsRscMgr() override { return *this; }
};

template<typename TResource>
const std::string &GetCompletePoolName()
{
    static const std::string ret = std::string("pool.") + TResource::GetPoolName();
    return ret;
}

// 直接根据参数创建一个对象
// params一定不是引用
template<typename TResource>
std::shared_ptr<TResource> CreateResourceFromScratch(ResourceManager &rscMgr, std::string instanceName, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const IResource::ImportContext &ctx)
{
    auto &type = params["type"].AsValue();
    auto instance = rscMgr.GetCreatorManager<TResource>().Create(rscMgr, type, std::move(instanceName));
    instance->Import(rscMgr, root, params, ctx);
    return instance;
}

// 用名字取得一个池子中的对象
// 这个对象一定不是引用，可以直接创建
// 如果池子中已经有了，就直接返回
// 否则，根据参数来创建并存进池子里
template<typename TResource>
std::shared_ptr<TResource> GetResourceInPool(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const std::string &rscName, const IResource::ImportContext &ctx)
{
    auto &pool = rscMgr.GetPool<TResource>();
    if(auto instance = pool.GetByName(rscName))
        return instance;

    auto &poolName = GetCompletePoolName<TResource>();
    std::string paramPath = poolName + "." + rscName;
    auto newInstance = CreateResourceFromScratch<TResource>(rscMgr, rscName, root, root[paramPath].AsGroup(), ctx);
    pool.AddInstance(newInstance);

    return newInstance;
}

// 导入某种类型的池子中的所有数据
template<typename TResource>
void ImportFromPool(const AGZ::ConfigGroup &root, ResourceManager &rscMgr, const IResource::ImportContext &ctx)
{
    auto &pool = rscMgr.GetPool<TResource>();

    auto &children = root[GetCompletePoolName<TResource>()].AsGroup().GetChildren();
    for(auto &it : children)
        GetResourceInPool<TResource>(rscMgr, root, it.first, ctx);
}

// 对一个right item，不断解引用并返回最后的不是引用的group
inline const AGZ::ConfigGroup &GetFinalNonReferenceParam(const AGZ::ConfigGroup &root, const AGZ::ConfigNode &node)
{
    if(auto pVal = node.TryAsValue())
        return GetFinalNonReferenceParam(root, root[*pVal]);

    auto &grp = node.AsGroup();
    if(grp["type"].AsValue() == "Reference")
        return GetFinalNonReferenceParam(root, root[grp["name"].AsValue()]);

    return grp;
}

// 取得一个resource对象
// 如果该对象是一个目标为池子中对象的引用，那么向池子索要
// 否则，如果该对象是一个引用，递归地处理其解引用后的结果
// 否则，原地用参数创建匿名对象
template<typename TResource>
std::shared_ptr<TResource> GetResourceInstance(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigNode &params, const IResource::ImportContext &ctx)
{
    if(auto pVal = params.TryAsValue()) // 名字必然是引用
    {
        auto &poolName = GetCompletePoolName<TResource>();
        if(AGZ::StartsWith(*pVal, poolName + ".")) // 这是一个针对池子中的对象的引用
            return GetResourceInPool<TResource>(rscMgr, root, pVal->substr(poolName.length() + 1), ctx);

        // 针对不在池子中的引用，递归地调用自身
        auto &referencedParams = root[*pVal];
        return GetResourceInstance<TResource>(rscMgr, root, referencedParams, ctx);
    }

    auto &grp = params.AsGroup();
    if(grp["type"].AsValue() == "Reference") // 另一种引用类型
    {
        auto &name = grp["name"].AsValue();

        auto &poolName = GetCompletePoolName<TResource>();
        if(AGZ::StartsWith(name, poolName + ".")) // 这是一个针对池子中的对象的引用
            return GetResourceInPool<TResource>(rscMgr, root, name.substr(poolName.length() + 1), ctx);

        // 针对不在池子中的引用，递归地调用自身
        auto &referencedParams = root[name];
        return GetResourceInstance<TResource>(rscMgr, root, referencedParams, ctx);
    }

    // 直接创建匿名对象
    return CreateResourceFromScratch<TResource>(rscMgr, "", root, grp, ctx);
}

template<typename TResource, bool THasPool, bool TAllowAnonymous>
class TResourceSlot
{
    std::shared_ptr<TResource> instance_;

    TResourceCreatorSelector<TResource> *GetSelector(ResourceManager &rscMgr)
    {
        static std::unique_ptr<TResourceCreatorSelector<TResource>> anonymousSelector;
        if(!anonymousSelector)
            anonymousSelector = std::make_unique<TResourceCreatorSelector<TResource>>(rscMgr.GetCreatorManager<TResource>());
        return anonymousSelector.get();
    }

    std::shared_ptr<TResource> ShowNewAnonymousInstanceWindow(ResourceManager &rscMgr)
    {
        if(!ImGui::BeginPopup("new anonymous instance", ImGuiWindowFlags_AlwaysAutoResize))
            return nullptr;
        AGZ::ScopeGuard popupExitGuard([] { ImGui::EndPopup(); });

        auto *selector = GetSelector(rscMgr);
        selector->Display("type");

        if(ImGui::Button("ok") && selector->GetSelectedCreator())
        {
            ImGui::CloseCurrentPopup();
            return selector->GetSelectedCreator()->Create(rscMgr, "");
        }

        ImGui::SameLine();

        if(ImGui::Button("cancel"))
        {
            ImGui::CloseCurrentPopup();
            return nullptr;
        }

        return nullptr;
    }

public:

    std::shared_ptr<TResource> GetInstance()
    {
        return instance_;
    }

    void SetInstance(std::shared_ptr<TResource> instance)
    {
        instance_ = std::move(instance);
    }

    void Display(ResourceManager &rscMgr)
    {
        if constexpr(THasPool)
        {
            auto &pool = rscMgr.GetPool<TResource>();
            if(ImGui::SmallButton("&##set_instance_as_selected_one") && pool.GetSelectedInstance())
            {
                instance_ = pool.GetSelectedInstance();
            }
            ImGui::SameLine();
        }

        if constexpr(TAllowAnonymous)
        {
            if(ImGui::SmallButton("?##create_new_anonymous_instance"))
                ImGui::OpenPopup("new anonymous instance");
            ImGui::SameLine();
        }

        {
            const char *type = instance_ ? instance_->GetTypeName().c_str() : "?";
            const char *text = instance_ ? instance_->GetName().c_str() : "null";
            ImGui::TextWrapped("[%s] %s", type, text);
        }

        if constexpr(TAllowAnonymous)
        {
            if(auto anonymousInstance = ShowNewAnonymousInstanceWindow(rscMgr))
            {
                instance_ = std::move(anonymousInstance);
            }
        }

        if(instance_)
            instance_->Display(rscMgr);
    }

    void Export(const ResourceManager &rscMgr, SceneExportingContext &ctx) const
    {
        if(!instance_)
            throw std::runtime_error("empty resource instance");
        instance_->template ExportAsReference<TResource>(rscMgr, ctx);
    }

    void Import(const ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params)
    {
        instance_ = GetResourceInstance<TResource>(rscMgr, root, params);
    }
};

using CameraSlot                = TResourceSlot<CameraInstance,                true,  false>;
using EntitySlot                = TResourceSlot<EntityInstance,                true,  false>;
using FilmFilterSlot            = TResourceSlot<FilmFilterInstance,            false, true>;
using FresnelSlot               = TResourceSlot<FresnelInstance,               false, true>;
using GeometrySlot              = TResourceSlot<GeometryInstance,              true,  true>;
using LightSlot                 = TResourceSlot<LightInstance,                 true,  false>;
using MaterialSlot              = TResourceSlot<MaterialInstance,              true,  true>;
using PathTracingIntegratorSlot = TResourceSlot<PathTracingIntegratorInstance, false, true>;
using RendererSlot              = TResourceSlot<RendererInstance,              false, true>;
using SamplerSlot               = TResourceSlot<SamplerInstance,               false, true>;
using TextureSlot               = TResourceSlot<TextureInstance,               true,  true>;

void RegisterResourceCreators(ResourceManager &rscMgr);
