#include <algorithm>

#include <AGZUtils/Utils/Misc.h>

#include "GL.h"
#include "ModelDataManager.h"

void ModelDataManager::Display(Console &console)
{
    if(!ImGui::CollapsingHeader("Model Data Manager"))
        return;

    if(ImGui::Button("load"))
        ImGui::OpenPopup("Load From File");
    LoadFromFile(console);

    ImGui::SameLine();

    if(ImGui::Button("delete") && selectedIdx_ >= 0)
    {
        data_.erase(data_.begin() + selectedIdx_);
        if(selectedIdx_ >= static_cast<int>(data_.size()))
            --selectedIdx_;
    }

    ImGui::SameLine();

    if(ImGui::Checkbox("sort by name", &sortDataByName_) && sortDataByName_)
        SortData();

    ImGui::Separator();

    int count = static_cast<int>(data_.size());
    for(int i = 0; i < count; ++i)
    {
        ImGui::PushID(i);
        bool selected = selectedIdx_ == i;
        if(ImGui::Selectable(data_[i].nameText.c_str(), selected))
        {
            if(selected)
                selectedIdx_ = -1;
            else
                selectedIdx_ = i;
        }
        ImGui::PopID();
    }
}

const ModelDataManager::MeshGroupData *ModelDataManager::GetSelectedMeshGroup() const
{
    return selectedIdx_ >= 0 ? &data_[selectedIdx_] : nullptr;
}

bool ModelDataManager::Add(const AGZ::Str8 &name, MeshGroup &&meshGroup)
{
    // 禁止名字重复

    for(auto &d : data_)
    {
        if(d.name == name)
            return false;
    }

    // 新元素插入

    auto bb = meshGroup.GetBoundingBox();
    data_.push_back({ name, name.ToStdString(), std::move(meshGroup), bb });

    if(sortDataByName_)
        SortData();

    // 自动选择新插入的数据

    for(size_t i = 0; i < data_.size(); ++i)
    {
        if(data_[i].name == name)
        {
            selectedIdx_ = static_cast<int>(i);
            break;
        }
    }

    return true;
}

void ModelDataManager::LoadFromFile(Console &console)
{
    if(!ImGui::BeginPopupModal("Load From File", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        return;
    AGZ::ScopeGuard popupExitGuard([]() { ImGui::EndPopup(); });

    bool ok = false, cancel = false;

    static char nameBuf[256] = "default-name";
    ImGui::InputText("name", nameBuf, 256);

    static char filenameBuf[256] = "Scene/Asset/Test/Mitsuba.obj";
    ImGui::InputText("filename", filenameBuf, 256);

    ok |= ImGui::Button("load"); ImGui::SameLine();
    cancel |= ImGui::Button("cancel");

    if(ok)
    {
        AGZ::Mesh::WavefrontObj<float> obj;
        if(!obj.LoadFromFile(filenameBuf))
        {
            console.AddText(ConsoleText::Error, "Failed to load obj data from " + std::string(filenameBuf));
            ImGui::CloseCurrentPopup();
            return;
        }

        MeshGroup meshGrp = obj.ToGeometryMeshGroup();
        obj.Clear();

        if(!Add(nameBuf, std::move(meshGrp)))
        {
            console.AddText(ConsoleText::Error, "Failed to add new model data: " + std::string(nameBuf));
            ImGui::CloseCurrentPopup();
            return;
        }

        console.AddText(ConsoleText::Normal, "Load WavefrontOBJ from " + std::string(filenameBuf));
        console.AddText(ConsoleText::Normal, "Add model data: " + std::string(nameBuf));
        ImGui::CloseCurrentPopup();
        return;
    }

    if(cancel)
        ImGui::CloseCurrentPopup();
}

void ModelDataManager::SortData()
{
    std::sort(
        data_.begin(), data_.end(),
        [](auto &L, auto &R) { return L.name < R.name; });
}
