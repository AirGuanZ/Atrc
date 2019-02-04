#include "GL.h"
#include "MaterialManager.h"

MaterialCreatorTable::MaterialCreatorTable()
{
    
}

const MaterialCreator &MaterialCreatorTable::operator[](size_t idx) const
{
    AGZ_ASSERT(idx < GetTableSize());
    return *creators_[idx];
}

MaterialCreatorSelector::MaterialCreatorSelector()
{
    auto &table = MaterialCreatorTable::GetInstance();

    if(table.GetTableSize())
        selectedMaterialCreator_ = &table[0];
    else
        selectedMaterialCreator_ = nullptr;
}

void MaterialCreatorSelector::Display(const char *label)
{
    auto &table = MaterialCreatorTable::GetInstance();

    const char *selectedCreatorName = selectedMaterialCreator_ ? selectedMaterialCreator_->GetName() : nullptr;
    if(ImGui::BeginCombo(label, selectedCreatorName))
    {
        for(size_t i = 0; i < table.GetTableSize(); ++i)
        {
            bool isSelected = selectedCreatorName == table[i].GetName();
            if(ImGui::Selectable(table[i].GetName(), isSelected))
                selectedMaterialCreator_ = &table[i];
            if(isSelected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
}

const MaterialCreator *MaterialCreatorSelector::GetSelectedMaterialCreator() const
{
    return selectedMaterialCreator_;
}

MaterialPool::MaterialPool()
{
    selectedMaterialIndex_ = INDEX_NONE;
}

void MaterialPool::Display()
{
    if(!ImGui::BeginChild("material list##all_material_instances"))
        return;
    AGZ::ScopeGuard exitChildGuard([] { ImGui::EndChild(); });

    for(size_t i = 0; i < materials_.size(); ++i)
    {
        ImGui::PushID(static_cast<int>(i));

        bool isSelected = i == selectedMaterialIndex_;
        if(ImGui::Selectable(materials_[i]->GetName().c_str(), isSelected))
        {
            if(isSelected)
                selectedMaterialIndex_ = INDEX_NONE;
            else
                selectedMaterialIndex_ = i;
        }

        ImGui::PopID();
    }
}

std::shared_ptr<const MaterialInstance> MaterialPool::GetSelectedMaterial() const
{
    return selectedMaterialIndex_ == INDEX_NONE ? materials_[selectedMaterialIndex_] : std::shared_ptr<const MaterialInstance>();
}
