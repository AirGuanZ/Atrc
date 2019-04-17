#pragma once

#include <Atrc/Editor/ResourceInstance/ResourceCreatorSelector.h>

namespace Atrc::Editor
{

template<typename TResourceFactory>
class ResourceSlot
{
public:

    using Factory = TResourceFactory;
    using Creator = typename Factory::Creator;
    using Resource = typename Creator::Resource;

    ResourceSlot()
        : asDnDSource_(true), asDnDShareTarget_(false), asDnDCloneTarget_(true)
    {
        if(auto c = selector_.GetSelectedCreator())
            rsc_ = c->Create();
    }

    template<typename TCallback>
    void SetResourceChangedCallback(TCallback &&callback, bool applyRightAway = true)
    {
        rscChangedCallback_ = std::forward<TCallback>(callback);
        if(applyRightAway && rsc_)
            rscChangedCallback_(*rsc_);
    }

    void SetResource(std::shared_ptr<Resource> rsc)
    {
        rsc_ = std::move(rsc);
        selector_.SetSelectedCreator(rsc_ ? rsc_->GetType() : "");
        if(rscChangedCallback_ && rsc_)
            rscChangedCallback_(*rsc_);
    }

    void ClearResourceChangedCallback()
    {
        rscChangedCallback_ = nullptr;
    }

    void Display()
    {
        bool useMiniWidth = true; //rsc_ ? !CallIsMultiline(*rsc_) : false;
        if(selector_.Display(useMiniWidth))
        {
            rsc_ = selector_.GetSelectedCreator()->Create();
            if(rscChangedCallback_)
                rscChangedCallback_(*rsc_);
        }
        
        AsDnDSource();
        AsDnDTarget();

        if(rsc_)
        {
            if(!rsc_->IsMultiline())
                ImGui::SameLine();
            ImGui::PushID(rsc_.get());
            AGZ_SCOPE_GUARD({ ImGui::PopID(); });
            rsc_->Display();
        }
    }

    void DisplayAsSubresource(const char *attribName)
    {
        if(attribName)
            ImGui::TextUnformatted(attribName);
        if(IsMultiline())
        {
            if(attribName)
                ImGui::Indent();
            Display();
            if(attribName)
                ImGui::Unindent();
        }
        else
        {
            ImGui::SameLine();
            Display();
        }
    }

    std::shared_ptr<Resource> GetResource() const noexcept
    {
        return rsc_;
    }

    std::shared_ptr<Resource> GetNoneNullResource() const
    {
        if(rsc_)
            return rsc_;
        throw std::runtime_error("GetNoneNullResource is called with a null resource slot");
    }

    bool IsMultiline() const noexcept
    {
        return rsc_ ? CallIsMultiline(*rsc_) : false;
    }

    void AsDnDSource(bool asDnDSource) noexcept { asDnDSource_ = asDnDSource; }
    void AsDnDShareTarget(bool asDnDTarget) noexcept { if((asDnDShareTarget_ = asDnDTarget)) asDnDCloneTarget_ = false; }
    void AsDnDCloneTarget(bool asDnDTarget) noexcept { if((asDnDCloneTarget_ = asDnDTarget)) asDnDShareTarget_ = false; }

    bool IsDnDSource() const noexcept { return asDnDSource_; }
    bool IsDnDShareTarget() const noexcept { return asDnDShareTarget_; }
    bool IsDnDCloneTarget() const noexcept { return asDnDCloneTarget_; }

private:

    ResourceCreatorSelector<Factory> selector_;
    std::shared_ptr<Resource> rsc_;

    std::function<void(Resource&)> rscChangedCallback_;

    bool asDnDSource_;
    bool asDnDShareTarget_;
    bool asDnDCloneTarget_;

    void AsDnDSource()
    {
        if(!asDnDSource_ || !ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
            return;
        AGZ_SCOPE_GUARD({ ImGui::EndDragDropSource(); });
        auto pRsc = &rsc_;
        ImGui::SetDragDropPayload(Resource::StrID(), &pRsc, sizeof(pRsc));

        if(rsc_)
            rsc_->DisplayOnDnD();
        else
            ImGui::Text("null");
    }

    void AsDnDTarget()
    {
        if(!asDnDShareTarget_ && !asDnDCloneTarget_)
            return;
        if(!ImGui::BeginDragDropTarget())
            return;
        AGZ_SCOPE_GUARD({ ImGui::EndDragDropTarget(); });

        auto payload = ImGui::AcceptDragDropPayload(Resource::StrID());
        if(!payload)
            return;

        AGZ_ASSERT(payload->Data);

        std::shared_ptr<Resource> *pRsc;
        AGZ_ASSERT(payload->DataSize == sizeof(pRsc));
        std::memcpy(&pRsc, payload->Data, sizeof(pRsc));

        auto rsc = *pRsc;
        if(rsc && asDnDCloneTarget_)
            SetResource(rsc->Clone());
        else
            SetResource(rsc);
    }

    template<typename T, typename = void>
    struct HasIsMultiline : std::false_type { };

    template<typename T>
    struct HasIsMultiline<T, std::void_t<decltype(std::declval<const T&>().IsMultiline())>> : std::true_type { };

    template<typename T, bool THasMultiline>
    struct CallIsMultilineAux { static bool Call(const T &v) noexcept { return true; } };

    template<typename T>
    struct CallIsMultilineAux<T, true> { static bool Call(const T &v) noexcept { return v.IsMultiline(); } };

    static bool CallIsMultiline(const Resource &t) noexcept
    {
        return CallIsMultilineAux<Resource, HasIsMultiline<Resource>::value>::Call(t);
    }
};

}; // namespace Atrc::Editor
