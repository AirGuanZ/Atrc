#pragma once

#include <algorithm>

#include <Atrc/Editor/GL.h>
#include <Atrc/Editor/ResourceInstance/ResourceInstance.h>

template<typename TResourceFactory>
class ResourceCreatorSelector
{
    const TResourceFactory &factory_;
    const typename TResourceFactory::Creator *selectedCreator_;
    float widgetWidth_;

public:

    using Factory = TResourceFactory;
    using Creator = typename Factory::Creator;
    using Resource = typename Creator::Resource;

    static_assert(std::is_base_of_v<IResourceCreator, Creator>);
    static_assert(std::is_base_of_v<IResource, Resource>);

    explicit ResourceCreatorSelector(const Factory &factory) noexcept
        : factory_(factory)
    {
        AGZ_ASSERT(factory.begin() != factory.end());
        selectedCreator_ = factory.begin()->second;

        widgetWidth_ = 0;
        for(auto &p : factory)
            widgetWidth_ = (std::max)(widgetWidth_, ImGui::CalcTextSize(p.first.c_str()).x);
        widgetWidth_ += 2 * ImGui::GetFontSize();
    }

    void SetSelectedCreator(const std::string &name)
    {
        selectedCreator_ = factory_[name];
    }

    const Creator *GetSelectedCreator() const noexcept
    {
        return selectedCreator_;
    }

    bool Display(bool miniWidth)
    {
        ImGui::PushID(this);
        AGZ_SCOPE_GUARD({ ImGui::PopID(); });

        ImGui::PushItemWidth(miniWidth ? widgetWidth_ : -1.0f);
        AGZ_SCOPE_GUARD({ ImGui::PopItemWidth(); });

        auto oldSelectedCreator = selectedCreator_;

        if(ImGui::BeginCombo("", selectedCreator_->GetName().c_str()))
        {
            AGZ_SCOPE_GUARD({ ImGui::EndCombo(); });
            for(auto &p : factory_)
            {
                bool selected = selectedCreator_ == p.second;
                if(ImGui::Selectable(p.first.c_str(), selected))
                    selectedCreator_ = p.second;
            }
        }

        return selectedCreator_ != oldSelectedCreator;
    }
};
