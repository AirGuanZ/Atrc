#pragma once

#include <Atrc/Atrc/ResourceInterface/ResourceInstance.h>
#include <Atrc/Atrc/GL.h>

template<typename TBase>
class ResourceCreatorSelector
{
    static_assert(std::is_base_of_v<ResourceInstance, TBase>);

    const ResourceCreatorManager<TBase> creatorMgr_;
    const ResourceCreator<TBase> *selectedCreator_;

public:

    explicit ResourceCreatorSelector(const ResourceCreatorManager<TBase> &creatorMgr) noexcept
        : creatorMgr_(creatorMgr)
    {
        AGZ_ASSERT(creatorMgr.begin() != creator.end());
        selectedCreator_ = creatorMgr.begin()->second;
    }

    void SetSelectedCreator(const std::string &name)
    {
        selectedCreator_ = creatorMgr_[name];
        AGZ_ASSERT(selectedCreator_);
    }

    const ResourceCreator<TBase> *GetSelectedCreator() const
    {
        return selectedCreator_;
    }

    bool Display()
    {
        auto oldSelectedCreator = selectedCreator_;

        if(ImGui::BeginCombo("", selectedCreator_->GetName()))
        {
            AGZ::ScopeGuard g([] { ImGui::EndCombo(); });
            for(auto &p : creatorMgr_)
            {
                bool selected = p.second == selectedCreator_;
                if(ImGui::Selectable(p.first.c_str(), selected))
                    selectedCreator_ = p.second;
            }
        }

        return selectedCreator_ != oldSelectedCreator;
    }
};
