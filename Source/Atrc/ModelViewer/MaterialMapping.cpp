#include <Atrc/ModelViewer/MaterialMapping.h>

namespace
{
    std::unique_ptr<MaterialCreatorSelector> anonymousMatTypeSelector;

    MaterialCreatorSelector *AnonymousSelector(ObjectManager &objMgr)
    {
        if(!anonymousMatTypeSelector)
        {
            anonymousMatTypeSelector = std::make_unique<MaterialCreatorSelector>(
                objMgr.GetCreatorManager<MaterialInstance>());
        }
        return anonymousMatTypeSelector.get();
    }

    std::shared_ptr<MaterialInstance> ShowNewAnonymousMaterialWindow(ObjectManager &objMgr)
    {
        if(!ImGui::BeginPopup("new anonymous material", ImGuiWindowFlags_AlwaysAutoResize))
            return nullptr;
        AGZ::ScopeGuard popupExitGuard([] { ImGui::EndPopup(); });

        auto *selector = AnonymousSelector(objMgr);
        selector->Display("type");

        if(ImGui::Button("ok") && selector->GetSelectedCreator())
        {
            ImGui::CloseCurrentPopup();
            return selector->GetSelectedCreator()->Create("anonymous");
        }

        ImGui::SameLine();

        if(ImGui::Button("cancel"))
        {
            ImGui::CloseCurrentPopup();
            return nullptr;
        }

        return nullptr;
    }

} // namespace null

void MaterialMappingSelector::Display(ObjectManager &objMgr)
{
    static const std::string MAPPING_TYPES[] =
    {
        "single",
    };

    const std::string *oldMappingTypeName = curMappingTypeName_;
    if(ImGui::BeginCombo("mapping", curMappingTypeName_ ? curMappingTypeName_->c_str() : nullptr))
    {
        for(size_t i = 0; i < AGZ::ArraySize(MAPPING_TYPES); ++i)
        {
            ImGui::PushID(static_cast<int>(i));
            bool isSelected = &MAPPING_TYPES[i] == curMappingTypeName_;
            if(ImGui::Selectable(MAPPING_TYPES[i].c_str(), isSelected))
                curMappingTypeName_ = &MAPPING_TYPES[i];
            if(isSelected)
                ImGui::SetItemDefaultFocus();
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }

    if(oldMappingTypeName != curMappingTypeName_)
    {
        if(curMappingTypeName_ == &MAPPING_TYPES[0])
            mapping_ = std::make_unique<SingleMaterialMapping>();
        else
            mapping_ = nullptr;
    }

    if(mapping_)
        mapping_->Display(objMgr);
}

void SingleMaterialMapping::Display(ObjectManager &objMgr)
{
    ImGui::Text("material: %s", material_ ? material_->GetName().c_str() : "null");

    auto &matPool = objMgr.GetPool<MaterialInstance>();
    if(ImGui::Button("set##set_material_as_selected_one") && matPool.GetSelectedInstance())
        material_ = matPool.GetSelectedInstance();

    ImGui::SameLine();

    if(ImGui::Button("new##create_new_anonymous_material"))
        ImGui::OpenPopup("new anonymous material");

    if(auto anonymousMat = ShowNewAnonymousMaterialWindow(objMgr))
        material_ = std::move(anonymousMat);

    if(material_)
        material_->Display();
}
