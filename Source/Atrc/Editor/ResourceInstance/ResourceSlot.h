#pragma once

#include <Atrc/Editor/ResourceInstance/ResourceCreatingContext.h>
#include <Atrc/Editor/ResourceInstance/ResourceCreatorSelector.h>

template<typename TResourceCategory>
class ResourceSlot
{
    ResourceCreatorSelector<TResourceCategory> selector_;
    std::shared_ptr<TResourceCategory> rsc_;

    std::function<void(TResourceCategory&)> rscChangedCallback_;
    ResourceCreatingContext &rscCreatingCtx_;

    template<typename T, typename = void>
    struct HasIsMultiline : std::false_type { };

    template<typename T>
    struct HasIsMultiline<T, std::void_t<decltype(std::declval<const T&>().IsMultiline())>> : std::true_type { };
    
    template<typename T, bool THasMultiline>
    struct CallIsMultilineAux { static bool Call(const T &v) noexcept { return true; } };

    template<typename T>
    struct CallIsMultilineAux<T, true> { static bool Call(const T &v) noexcept { return v.IsMultiline(); } };

    static bool CallIsMultiline(const TResourceCategory &t) noexcept
    {
        return CallIsMultilineAux<TResourceCategory, HasIsMultiline<TResourceCategory>::value>::Call(t);
    }

public:

    explicit ResourceSlot(ResourceCreatingContext &ctx)
        : selector_(ctx.GetFactory<TResourceCategory>()),
          rscCreatingCtx_(ctx)
    {

    }

    template<typename TCallback>
    void SetResourceChangedCallback(TCallback &&callback, bool applyRightAway = true)
    {
        rscChangedCallback_ = std::forward<TCallback>(callback);
        if(applyRightAway && rsc_)
            rscChangedCallback_(*rsc_);
    }

    void ClearResourceChangedCallback()
    {
        rscChangedCallback_ = nullptr;
    }

    template<typename...RscDisplayArgs>
    void Display(RscDisplayArgs&&...rscDisplayArgs)
    {
        bool useMiniWidth = rsc_ ? !CallIsMultiline(*rsc_) : false;
        if(selector_.Display(useMiniWidth))
        {
            rsc_ = selector_.GetSelectedCreator()->Create("", rscCreatingCtx_);
            if(rscChangedCallback_)
                rscChangedCallback_(*rsc_);
        }

        if(rsc_)
        {
            if(!rsc_->IsMultiline())
                ImGui::SameLine();
            rsc_->Display(std::forward<RscDisplayArgs>(rscDisplayArgs)...);
        }
    }

    TResourceCategory *GetResource() noexcept
    {
        return rsc_.get();
    }
};
