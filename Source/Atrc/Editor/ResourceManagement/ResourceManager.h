#pragma once

#include <algorithm>
#include <limits>
#include <map>
#include <string>

#include <AGZUtils/Utils/Misc.h>

#include <Atrc/Editor/LauncherScriptExportingContext.h>
#include <Atrc/Editor/GL.h>

/**
 * Instance管理体系
 * 
 * ObjectManager持有一系列InstanceCreatorManager和InstancePool
 * InstanceCreatorManager保存一组InstanceCreator的指针
 * InstancePool持有一组Instance
 */

class LauncherScriptExportingContext;
class ResourceManager;

class IResource : public AGZ::Uncopiable
{
    std::string instanceName_;
    bool isInPool_;

public:

    template<typename TSubResource>
    static void ExportSubResource(
        std::string left, const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx,
        const TSubResource &subrsc)
    {
        ctx.AddLine(std::move(left) + " = {");
        ctx.IncIndent();
        subrsc.Export(rscMgr, ctx);
        ctx.DecIndent();
        ctx.AddLine("};");
    }

    explicit IResource(std::string name) noexcept
        : instanceName_(std::move(name)), isInPool_(false)
    {
        
    }

    virtual ~IResource() = default;

    const std::string &GetName() const noexcept
    {
        return instanceName_;
    }

    bool IsInPool() const noexcept
    {
        return isInPool_;
    }

    void PutIntoPool() noexcept
    {
        isInPool_ = true;
    }

    virtual void Display(ResourceManager &rscMgr) = 0;

    virtual void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const = 0;
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

    std::shared_ptr<TResource> GetSelectedInstance()
    {
        return selectedInstance_;
    }

    bool Find(std::shared_ptr<TResource> rsc) const
    {
        return std::find(std::begin(instances_), std::end(instances_), rsc) != std::end(instances_);
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
                auto newInstance = creatorSelector_.GetSelectedCreator()->Create(
                    rscMgr, "[" + creatorSelector_.GetSelectedCreator()->GetName() + "] " + nameBuf);
                instances_.push_back(newInstance);
                newInstance->PutIntoPool();
                nameBuf[0] = '\0';
                if(sortInstanceByName_)
                    SortInstanceByName();
            }
        }

        ImGui::SameLine();

        if(ImGui::Button("delete") && selectedInstance_)
        {
            auto it = std::find(std::begin(instances_), std::end(instances_), selectedInstance_);
            AGZ_ASSERT(it != instances_.end());

            size_t idx = it - instances_.begin();
            instances_.erase(it);
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
            if(ImGui::Selectable(instance->GetName().c_str(), isSelected))
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

template<typename...TResourceRegisters>
class TResourceManager
{
    std::tuple<TResourceCreatorManager<typename TResourceRegisters::ResourceType>...> creatorMgrList_;

    struct DummyPool_t { template<typename...Args> explicit DummyPool_t(Args&&...) { } };
    template<typename TRegister>
    using Pool_t = std::conditional_t<TRegister::HasPool,
                                      TResourcePool<typename TRegister::ResourceType>,
                                      DummyPool_t>;
    std::tuple<Pool_t<TResourceRegisters>...> poolList_;

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
        : poolList_(_creatorMgr<typename TResourceRegisters::ResourceType>()...)
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
            AsRscMgr(), creatorName, "[" + creatorName + "]" + (instanceName.empty() ? "" : " ") + std::move(instanceName));
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

    virtual void Render(const Mat4f &projViewMat) = 0;

    virtual void DisplayTransform(const Mat4f &proj, const Mat4f &view) = 0;
};
using EntityCreator = TResourceCreator<EntityInstance>;

class FilmFilterInstance : public IResource { using IResource::IResource; };
using FilmFilterCreator = TResourceCreator<FilmFilterInstance>;

class FresnelInstance : public IResource { public: using IResource::IResource; };
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

    virtual std::shared_ptr<const GL::VertexBuffer<Vertex>> GetVertexBuffer() const = 0;

    virtual std::vector<std::string> GetSubmeshNames() const = 0;
};
using GeometryCreator = TResourceCreator<GeometryInstance>;

class LightInstance : public IResource { public: using IResource::IResource; };
using LightCreator = TResourceCreator<LightInstance>;

class MaterialInstance : public IResource { public: using IResource::IResource; };
using MaterialCreator = TResourceCreator<MaterialInstance>;

class PathTracingIntegratorInstance : public IResource { public: using IResource::IResource; };
using PathTracingIntegratorCreator = TResourceCreator<PathTracingIntegratorInstance>;

class RendererInstance : public IResource { public: using IResource::IResource; };
using RendererCreator = TResourceCreator<RendererInstance>;

class SamplerInstance : public IResource { public: using IResource::IResource; };
using SamplerCreator = TResourceCreator<SamplerInstance>;

class TextureInstance : public IResource { public: using IResource::IResource; };
using TextureCreator = TResourceCreator<TextureInstance>;

class ResourceManager : public TResourceManager<
    TResourceRegister<CameraInstance, true>,
    TResourceRegister<EntityInstance, true>,
    TResourceRegister<FilmFilterInstance, false>,
    TResourceRegister<FresnelInstance, false>,
    TResourceRegister<GeometryInstance, true>,
    TResourceRegister<LightInstance, true>,
    TResourceRegister<MaterialInstance, true>,
    TResourceRegister<PathTracingIntegratorInstance, false>,
    TResourceRegister<RendererInstance, false>,
    TResourceRegister<SamplerInstance, false>,
    TResourceRegister<TextureInstance, true>>
{
public:

    using TResourceManager::TResourceManager;

    ResourceManager &AsRscMgr() override { return *this; }
};

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
            if constexpr(THasPool)
                return selector->GetSelectedCreator()->Create(rscMgr, "[" + selector->GetSelectedCreator()->GetName() + "] anonymous");
            else
                return selector->GetSelectedCreator()->Create(rscMgr, "[" + selector->GetSelectedCreator()->GetName() + "]");
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
            auto text = instance_ ? instance_->GetName().c_str() : "null";
            ImGui::TextWrapped("%s", text);
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

    void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const
    {
        if(!instance_)
            throw std::runtime_error("empty resource instance");
        instance_->Export(rscMgr, ctx);
    }
};

using CameraSlot                = TResourceSlot<CameraInstance,                true,  false>;
using EntitySlot                = TResourceSlot<EntityInstance,                true,  false>;
using FilmFilterSlot            = TResourceSlot<FilmFilterInstance,            false, true>;
using FresnelSlot               = TResourceSlot<FresnelInstance,               false, true>;
using GeometrySlot              = TResourceSlot<GeometryInstance,              true,  true>;
using LightSlot                 = TResourceSlot<LightInstance, true, false>;
using MaterialSlot              = TResourceSlot<MaterialInstance,              true,  true>;
using PathTracingIntegratorSlot = TResourceSlot<PathTracingIntegratorInstance, false, true>;
using RendererSlot              = TResourceSlot<RendererInstance,              false, true>;
using SamplerSlot               = TResourceSlot<SamplerInstance,               false, true>;
using TextureSlot               = TResourceSlot<TextureInstance,               true,  true>;

void RegisterResourceCreators(ResourceManager &rscMgr);
