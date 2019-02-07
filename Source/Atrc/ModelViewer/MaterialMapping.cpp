#include <Atrc/ModelViewer/MaterialMapping.h>

void MaterialMappingSelector::Display(ResourceManager &rscMgr)
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
        mapping_->Display(rscMgr);
}

void SingleMaterialMapping::Display(ResourceManager &rscMgr)
{
    slot_.Display(rscMgr);
}
