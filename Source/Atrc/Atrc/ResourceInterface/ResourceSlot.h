#pragma once

#include <functional>

#include <Atrc/Atrc/ResourceInterface/ResourceCreatorSelector.h>

template<typename TBase>
class ResourceSlot
{
    ResourceCreatorSelector<TBase> creatorSelector_;
    std::unique_ptr<TBase> rsc_;
    std::function<void(TBase&)> rscTypeChangedCallback_;

public:

    static void DoNothing(TBase&) { }

    explicit ResourceSlot(ResourceCreateContext &ctx, const std::string &selectedCreatorName = "")
        : creatorSelector_(ctx.ctrMgrList->GetCreatorMgr<TBase>())
    {
        if(!selectedCreatorName.empty())
            creatorSelector_.SetSelectedCreator(selectedCreatorName);
        rsc_ = creatorSelector_.GetSelectedCreator()->Create(ctx, "anonymous object");
    }

    template<typename TRscTypeChangedCallback>
    void SetRscTypeChangedCallback(TRscTypeChangedCallback rscTypeChangedCallback, bool applyRightAway = false)
    {
        rscTypeChangedCallback_ = rscTypeChangedCallback;
        if(applyRightAway)
            rscTypeChangedCallback_(*rsc_);
    }

    template<typename TRscTypeChangedCallback = decltype(&ResourceSlot<TBase>::DoNothing)>
    void Display(ResourceCreateContext &ctx, const TRscTypeChangedCallback &rscTypeChangedCallback = &ResourceSlot<TBase>::DoNothing)
    {
        AGZ_ASSERT(rsc_);
        if(creatorSelector_.Display())
        {
            rsc_ = creatorSelector_.GetSelectedCreator()->Create(ctx, "anonymous object");
            rscTypeChangedCallback(*rsc_);
            if(rscTypeChangedCallback_)
                rscTypeChangedCallback_(*rsc_);
        }
        if(!rsc_->IsMultiline())
            ImGui::SameLine();
        rsc_->Display(ctx);
    }

    TBase &GetContainedResource() noexcept
    {
        AGZ_ASSERT(rsc_);
        return *rsc_;
    }
};
