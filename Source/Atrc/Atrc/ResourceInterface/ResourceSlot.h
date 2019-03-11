#pragma once

#include <Atrc/Atrc/ResourceInterface/ResourceCreatorSelector.h>

template<typename TBase>
class ResourceSlot
{
    ResourceCreatorSelector<TBase> creatorSelector_;
    std::unique_ptr<TBase> rsc_;

public:

    explicit ResourceSlot(ResourceCreateContext &ctx, const std::string &selectedCreatorName = "")
        : creatorSelector_(ctx.ctrMgrList->GetCreatorMgr<TBase>())
    {
        if(!selectedCreatorName.empty())
            creatorSelector_.SetSelectedCreator(selectedCreatorName);
        rsc_ = creatorSelector_.GetSelectedCreator()->Create(ctx, "anonymous object");
    }

    void Display(ResourceCreateContext &ctx)
    {
        AGZ_ASSERT(rsc_);
        ImGui::PushItemWidth(100);
        if(creatorSelector_.Display())
            rsc_ = creatorSelector_.GetSelectedCreator()->Create(ctx, "anonymous object");
        ImGui::PopItemWidth();
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
