#include "Model.h"
#include "ModelManager.h"

ModelManager::ModelManager()
    : selectedIdx_(INDEX_NONE)
{
    
}

void ModelManager::Render(const Camera &camera) const
{
    if(models_.empty())
        return;

    Model::BeginRendering();

    for(auto &model : models_)
        model.Render(camera);

    Model::EndRendering();
}

void ModelManager::Display(Console &console, const Camera &camera)
{
    if(ImGui::CollapsingHeader("data"))
        dataMgr_.Display(console);

    if(!ImGui::CollapsingHeader("model"))
        return;

    bool clickNew = false;
    if(ImGui::Button("new##new_model_from_data"))
    {
        ImGui::OpenPopup("new model from data");
        clickNew = true;
    }

    NewModelFromData(console, clickNew);

    ImGui::SameLine();

    if(ImGui::Button("delete##delete_model") && selectedIdx_ != INDEX_NONE)
    {
        models_.erase(models_.begin() + selectedIdx_);
        if(selectedIdx_ >= models_.size())
        {
            if(models_.empty())
                selectedIdx_ = INDEX_NONE;
            else
                --selectedIdx_;
        }
    }

    for(size_t i = 0; i < models_.size(); ++i)
    {
        ImGui::PushID(static_cast<int>(i));

        bool selected = i == selectedIdx_;
        if(ImGui::Selectable(models_[i].GetName().c_str(), selected))
        {
            if(selected)
                selectedIdx_ = INDEX_NONE;
            else
                selectedIdx_ = i;
        }

        ImGui::PopID();
    }

    if(selectedIdx_ != INDEX_NONE)
    {
        models_[selectedIdx_].DisplayProperty();
        models_[selectedIdx_].DisplayTransform(camera);
    }
}

void ModelManager::NewModelFromData(Console &console, bool clickNew)
{
    if(!ImGui::BeginPopup("new model from data", ImGuiWindowFlags_AlwaysAutoResize))
        return;
	AGZ::ScopeGuard popupGuard([]() { ImGui::EndPopup(); });

    static char nameBuf[256];
    if(clickNew)
        nameBuf[0] = '\0';
    ImGui::InputText("name", nameBuf, 256);

    ImGui::SameLine();

    if(ImGui::Button("ok"))
    {
        AGZ::ScopeGuard closePopupGuard([]() { ImGui::CloseCurrentPopup(); });

        // 名字不能有重复

        std::string name = nameBuf;
        for(auto &m : models_)
        {
            if(m.GetName() == name)
            {
                console.AddError("Name already used");
                return;
            }
        }

        // 必须已经选中了一项模型数据

        auto data = dataMgr_.GetSelectedMeshGroup();
        if(!data)
        {
            console.AddError("No model data is selected");
            return;
        }
        auto vtxBuf = data->GetVertexBuffer();
        AGZ_ASSERT(vtxBuf);

        Model newModel(std::move(name));
        newModel.Initialize(vtxBuf, Vec3f(0.5f));
        models_.push_back(std::move(newModel));
        selectedIdx_ = models_.size() - 1;

        console.AddMessage("Model created: " + std::string(nameBuf));
    }
}
