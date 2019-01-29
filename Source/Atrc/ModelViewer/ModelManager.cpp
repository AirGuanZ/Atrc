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

void ModelManager::Display(Console &console)
{
    if(ImGui::CollapsingHeader("data"))
        dataMgr_.Display(console);

    if(!ImGui::CollapsingHeader("model"))
        return;

    bool clickNew = false;
    if(ImGui::Button("new"))
    {
        ImGui::OpenPopup("new model from data");
        clickNew = true;
    }

    NewModelFromData(console, clickNew);

    for(size_t i = 0; i < models_.size(); ++i)
    {
        ImGui::PushID(int(i));

        if(ImGui::TreeNode(models_[i].GetName().c_str()))
        {
            models_[i].DisplayProperty();
            models_[i].DisplayTransformSeq();
            ImGui::TreePop();
        }

        ImGui::PopID();
    }
}

void ModelManager::NewModelFromData(Console &console, bool clickNew)
{
    if(!ImGui::BeginPopup("new model from data", ImGuiWindowFlags_AlwaysAutoResize))
        return;
	AGZ::ScopeGuard popupGuard([]() { ImGui::EndPopup(); });

    static char nameBuf[256] = "";
    if(clickNew)
        nameBuf[0] = '\0';

    ImGui::InputText("name", nameBuf, 256);

    ImGui::SameLine();

    if(ImGui::Button("ok"))
    {
        AGZ::ScopeGuard closePopupGuard([]() { ImGui::CloseCurrentPopup(); });

        std::string name = nameBuf;
        for(auto &m : models_)
        {
            if(m.GetName() == name)
            {
                console.AddError("Name already used");
                return;
            }
        }

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
