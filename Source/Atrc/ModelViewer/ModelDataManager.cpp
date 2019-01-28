#include <algorithm>
#include <cstdint>

#include <AGZUtils/Utils/Misc.h>

#include "GL.h"
#include "ModelDataManager.h"

std::shared_ptr<const GL::VertexBuffer<Model::Vertex>> ModelDataManager::MeshGroupData::GetVertexBuffer() const
{
    if(vtxBuf)
        return vtxBuf;

    std::shared_ptr<GL::VertexBuffer<Model::Vertex>> buf = std::make_shared<GL::VertexBuffer<Model::Vertex>>();
    std::vector<Model::Vertex> vtxData;

    for(auto &it : meshGroup.submeshes)
    {
        for(auto &v : it.second.vertices)
        {
            Model::Vertex nv;
            nv.pos = v.pos;
            nv.nor = v.nor;
            vtxData.push_back(nv);
        }
    }

    buf->InitializeHandle();
    buf->ReinitializeData(vtxData.data(), static_cast<uint32_t>(vtxData.size()), GL_STATIC_DRAW);
    vtxBuf = std::move(buf);

    return vtxBuf;
}

void ModelDataManager::Display(Console &console)
{
    bool newPopup = false;
    if(ImGui::Button("load"))
    {
        ImGui::OpenPopup("new data from .obj");
        newPopup = true;
    }
    //LoadFromFile(console, newPopup);
    DisplayNewData(console, newPopup);

    ImGui::SameLine();

    /*if(ImGui::Button("files"))
    {
        ImGui::OpenPopup("file browser");
        fileBrowser_.SetLabel("file browser");
        fileBrowser_.SetTarget(false);
        fileBrowser_.SetCurrentDirectory();
    }

    if(fileBrowser_.Display())
    {
        // TODO
    }

    ImGui::SameLine();*/

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
        if(ImGui::Selectable(data_[i].name.c_str(), selected))
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

bool ModelDataManager::Add(std::string_view name, MeshGroup &&meshGroup)
{
    // 禁止名字重复

    for(auto &d : data_)
    {
        if(d.name == name)
            return false;
    }

    // 新元素插入

	MeshGroupData grpData;
    grpData.bounding  = meshGroup.GetBoundingBox();
    grpData.meshGroup = std::move(meshGroup);
    grpData.name      = name;
    data_.push_back(std::move(grpData));

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

namespace
{
    std::string DefaultName(size_t idx)
    {
        return std::string("default-") + std::to_string(idx);
    }
}

void ModelDataManager::DisplayNewData(Console &console, bool newPopup)
{
    if(!ImGui::BeginPopup("new data from .obj", ImGuiWindowFlags_AlwaysAutoResize))
        return;
    AGZ::ScopeGuard popupExitGuard([] { ImGui::EndPopup(); });

    static char nameBuf[512] = "";
    if(newPopup)
    {
        fileBrowser_.SetLabel("file browser");
        fileBrowser_.SetTarget(false);
        fileBrowser_.SetCurrentDirectory();
        std::strcpy(nameBuf, DefaultName(defaultNameIndex_).c_str());
    }

    ImGui::InputText("name", nameBuf, 512);
    ImGui::Text("%s", fileBrowser_.GetResult().c_str());

    ImGui::SameLine();

    if(ImGui::Button("browse"))
        ImGui::OpenPopup("file browser");
    fileBrowser_.Display();

    if(ImGui::Button("ok"))
    {
        LoadObj(console, fileBrowser_.GetResult(), nameBuf);
        ImGui::CloseCurrentPopup();
        return;
    }

    ImGui::SameLine();

    if(ImGui::Button("cancel"))
        ImGui::CloseCurrentPopup();
}

void ModelDataManager::LoadObj(Console &console, std::string_view filename, std::string_view dataName)
{
    AGZ::Mesh::WavefrontObj<float> obj;
    if(!obj.LoadFromFile(filename))
    {
        console.AddText(ConsoleText::Error, "Failed to load obj data from " + std::string(filename));
        return;
    }

    MeshGroup meshGrp = obj.ToGeometryMeshGroup();
    obj.Clear();

    if(!Add(dataName, std::move(meshGrp)))
    {
        console.AddText(ConsoleText::Error, "Failed to add new model data: " + std::string(dataName));
        return;
    }

    if(dataName == DefaultName(defaultNameIndex_))
        ++defaultNameIndex_;

    console.AddText(ConsoleText::Normal, "Load WavefrontOBJ from " + std::string(filename));
    console.AddText(ConsoleText::Normal, "Add model data: " + std::string(dataName));
}

void ModelDataManager::SortData()
{
    std::sort(
        data_.begin(), data_.end(),
        [](auto &L, auto &R) { return L.name < R.name; });
}
