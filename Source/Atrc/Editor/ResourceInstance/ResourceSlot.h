#pragma once

#include <Atrc/Editor/ResourceInstance/ResourceCreatorSelector.h>

template<typename TResourceFactory, typename...CreatingArgs>
class ResourceSlot
{
public:

    using Factory = TResourceFactory;
    using Creator = typename Factory::Creator;
    using Resource = typename Creator::Resource;

    explicit ResourceSlot(const Factory &factory, CreatingArgs...creatingArgs)
        : selector_(factory), creatingArgs_(std::make_tuple<CreatingArgs...>(creatingArgs...))
    {
        if(selector_.GetSelectedCreator())
        {
            rsc_ = std::apply(std::mem_fn(&Creator::Create),
                std::tuple_cat(std::make_tuple(selector_.GetSelectedCreator()), creatingArgs_));
        }
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

    template<typename...RscDisplayArgs>
    void Display(RscDisplayArgs&&...rscDisplayArgs)
    {
        bool useMiniWidth = rsc_ ? !CallIsMultiline(*rsc_) : false;
        if(selector_.Display(useMiniWidth))
        {
            rsc_ = std::apply(std::mem_fn(&Creator::Create),
                              std::tuple_cat(std::make_tuple(selector_.GetSelectedCreator()), creatingArgs_));
            if(rscChangedCallback_)
                rscChangedCallback_(*rsc_);
        }

        if(rsc_)
        {
            if(!rsc_->IsMultiline())
                ImGui::SameLine();
            ImGui::PushID(rsc_.get());
            AGZ_SCOPE_GUARD({ ImGui::PopID(); });
            rsc_->Display(std::forward<RscDisplayArgs>(rscDisplayArgs)...);
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
        throw AGZ::HierarchyException("GetNoneNullResource is called with a null resource slot");
    }

private:

    ResourceCreatorSelector<Factory> selector_;
    std::shared_ptr<Resource> rsc_;

    std::function<void(Resource&)> rscChangedCallback_;

    std::tuple<CreatingArgs...> creatingArgs_;

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
