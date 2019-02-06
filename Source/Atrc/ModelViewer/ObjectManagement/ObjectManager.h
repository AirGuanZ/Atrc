#pragma once

#include <algorithm>
#include <limits>
#include <string>
#include <unordered_map>

#include <AGZUtils/Utils/Misc.h>

#include <Atrc/ModelViewer/GL.h>

/**
 * Instance管理体系
 * 
 * ObjectManager持有一系列InstanceCreatorManager和InstancePool
 * InstanceCreatorManager保存一组InstanceCreator的指针
 * InstancePool持有一组Instance
 */

class ObjectManager;

class InstanceInterface : public AGZ::Uncopiable
{
    std::string instanceName_;

public:

    explicit InstanceInterface(std::string name) noexcept
        : instanceName_(std::move(name))
    {
        
    }

    virtual ~InstanceInterface() = default;

    const std::string &GetName() const noexcept
    {
        return instanceName_;
    }

    virtual void Display(ObjectManager &objMgr) = 0;
};

template<typename TInstance>
class TInstanceCreator : public AGZ::Uncopiable
{
    std::string creatorName_;

public:

    explicit TInstanceCreator(std::string name) noexcept
        : creatorName_(std::move(name))
    {
        
    }

    virtual ~TInstanceCreator() = default;

    virtual std::shared_ptr<TInstance> Create(std::string name) const = 0;

    const std::string &GetName() const noexcept
    {
        return creatorName_;
    }
};

template<typename TInstance>
class TInstanceCreatorManager
{
    std::unordered_map<std::string, const TInstanceCreator<TInstance>*> name2Creator_;

public:

    std::shared_ptr<TInstance> Create(const std::string &creatorName, std::string instanceName) const
    {
        AGZ_ASSERT(name2Creator_.find(creatorName) != name2Creator_.end());
        return name2Creator_[creatorName]->Create(std::move(instanceName));
    }

    void AddCreator(const TInstanceCreator<TInstance> *creator)
    {
        AGZ_ASSERT(creator);
        name2Creator_[creator->GetName()] = creator;
    }

    auto begin()       { return name2Creator_.begin(); }
    auto end()         { return name2Creator_.end();   }
    auto begin() const { return name2Creator_.begin(); }
    auto end()   const { return name2Creator_.end();   }
};

template<typename TInstance, bool THasPool>
struct TInstanceRegister
{
    using InstanceType                   = TInstance;
    static constexpr bool HasPool        = THasPool;
};

template<typename TInstance>
class TInstanceCreatorSelector
{
    const TInstanceCreator<TInstance> *selectedCreator_;
    TInstanceCreatorManager<TInstance> &creatorMgr_;

public:

    explicit TInstanceCreatorSelector(TInstanceCreatorManager<TInstance> &mgr) noexcept
        : selectedCreator_(nullptr), creatorMgr_(mgr)
    {
        
    }

    const TInstanceCreator<TInstance> *GetSelectedCreator() noexcept
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

template<typename TInstance>
class TInstancePool
{
    static constexpr size_t INDEX_NONE = (std::numeric_limits<size_t>::max)();

    std::shared_ptr<TInstance> selectedInstance_;
    std::vector<std::shared_ptr<TInstance>> instances_;

    TInstanceCreatorSelector<TInstance> creatorSelector_;
    TInstanceCreatorManager<TInstance> &creatorMgr_;

    bool sortInstanceByName_;

    void SortInstanceByName()
    {
        std::sort(begin(instances_), end(instances_), [](auto &L, auto &R)
        {
            auto lL = AGZ::ToLower(L->GetName()), lR = AGZ::ToLower(R->GetName());
            return  lL < lR || (lL == lR && L->GetName() < R->GetName());
        });
    }

public:

    explicit TInstancePool(TInstanceCreatorManager<TInstance> &creatorMgr)
        : creatorSelector_(creatorMgr), creatorMgr_(creatorMgr), sortInstanceByName_(false)
    {
        
    }

    std::shared_ptr<TInstance> GetSelectedInstance()
    {
        return selectedInstance_;
    }

    void Display()
    {
        creatorSelector_.Display("type");

        static char nameBuf[256] = "";
        ImGui::InputText("name", nameBuf, 256);
        
        if(ImGui::Button("new") && creatorSelector_.GetSelectedCreator())
        {
            auto it = std::find_if(begin(instances_), end(instances_), [&](auto &c) { return c->GetName() == nameBuf; });
            if(it == instances_.end() && nameBuf[0])
            {
                instances_.push_back(creatorSelector_.GetSelectedCreator()->Create(
                    "[" + creatorSelector_.GetSelectedCreator()->GetName() + "] " + nameBuf));
                nameBuf[0] = '\0';
                if(sortInstanceByName_)
                    SortInstanceByName();
            }
        }

        ImGui::SameLine();

        if(ImGui::Button("delete") && selectedInstance_)
        {
            auto it = std::find(begin(instances_), end(instances_), selectedInstance_);
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

        ImGui::BeginChild("elems", ImVec2(0, 200), true);

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
};

template<typename...TInstanceRegisters>
class TObjectManager
{
    std::tuple<TInstanceCreatorManager<typename TInstanceRegisters::InstanceType>...> creatorMgrList_;

    struct DummyPool_t { template<typename...Args> explicit DummyPool_t(Args&&...) { } };
    template<typename TRegister>
    using Pool_t = std::conditional_t<TRegister::HasPool,
                                      TInstancePool<typename TRegister::InstanceType>,
                                      DummyPool_t>;
    std::tuple<Pool_t<TInstanceRegisters>...> poolList_;

    template<typename TInstance>
    TInstanceCreatorManager<TInstance> &_creatorMgr() noexcept
    {
        return std::get<TInstanceCreatorManager<TInstance>>(creatorMgrList_);
    }

    template<typename TInstance>
    TInstancePool<TInstance> &_pool() noexcept
    {
        return std::get<TInstancePool<TInstance>>(poolList_);
    }

public:

    TObjectManager()
        : poolList_(_creatorMgr<typename TInstanceRegisters::InstanceType>()...)
    {
        
    }

    template<typename TInstance>
    void AddCreator(const TInstanceCreator<TInstance> *creator)
    {
        _creatorMgr<TInstance>().AddCreator(std::move(creator));
    }

    template<typename TInstance>
    void Create(const std::string &creatorName, std::string instanceName) const
    {
        _creatorMgr<TInstance>().Create(creatorName, std::move(instanceName));
    }

    template<typename TInstance>
    TInstancePool<TInstance> &GetPool() noexcept
    {
        return _pool<TInstance>();
    }

    template<typename TInstance>
    TInstanceCreatorManager<TInstance> &GetCreatorManager() noexcept
    {
        return _creatorMgr<TInstance>();
    }
};

class MaterialInstance : public InstanceInterface { public: using InstanceInterface::InstanceInterface; };
using MaterialCreator = TInstanceCreator<MaterialInstance>;
using MaterialCreatorSelector = TInstanceCreatorSelector<MaterialInstance>;

class TextureInstance : public InstanceInterface { public: using InstanceInterface::InstanceInterface; };
using TextureCreator = TInstanceCreator<TextureInstance>;
using TextureCreatorSelector = TInstanceCreatorSelector<TextureInstance>;

class ObjectManager : public TObjectManager<
    TInstanceRegister<MaterialInstance, true>,
    TInstanceRegister<TextureInstance, true>>
{
public:

    using TObjectManager::TObjectManager;
};

template<typename TInstance>
class TInstanceSlot
{
    std::shared_ptr<TInstance> instance_;

    TInstanceCreatorSelector<TInstance> *GetSelector(ObjectManager &objMgr)
    {
        static std::unique_ptr<TInstanceCreatorSelector<TInstance>> anonymousSelector;
        if(!anonymousSelector)
            anonymousSelector = std::make_unique<TInstanceCreatorSelector<TInstance>>(objMgr.GetCreatorManager<TInstance>());
        return anonymousSelector.get();
    }

    std::shared_ptr<TInstance> ShowNewAnonymousInstanceWindow(ObjectManager &objMgr)
    {
        if(!ImGui::BeginPopup("new anonymous instance", ImGuiWindowFlags_AlwaysAutoResize))
            return nullptr;
        AGZ::ScopeGuard popupExitGuard([] { ImGui::EndPopup(); });

        auto *selector = GetSelector(objMgr);
        selector->Display("type");

        if(ImGui::Button("ok") && selector->GetSelectedCreator())
        {
            ImGui::CloseCurrentPopup();
            return selector->GetSelectedCreator()->Create("[" + selector->GetSelectedCreator()->GetName() + "] anonymous");
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

    void Display(ObjectManager &objMgr)
    {
        ImGui::Text("current: %s", instance_ ? instance_->GetName().c_str() : "null");

        auto &pool = objMgr.GetPool<TInstance>();
        if(ImGui::Button("set##set_instance_as_selected_one") && pool.GetSelectedInstance())
            instance_ = pool.GetSelectedInstance();

        ImGui::SameLine();

        if(ImGui::Button("new##create_new_anonymous_instance"))
            ImGui::OpenPopup("new anonymous instance");

        if(auto anonymousInstance = ShowNewAnonymousInstanceWindow(objMgr))
            instance_ = std::move(anonymousInstance);

        if(instance_)
            instance_->Display(objMgr);
    }
};

using MaterialSlot = TInstanceSlot<MaterialInstance>;
using TextureSlot  = TInstanceSlot<TextureInstance>;

void RegisterObjectCreators(ObjectManager &objMgr);
