#include "ModelManager.h"

ModelManager::ModelManager()
    : selectedIdx_(INDEX_NONE)
{
    
}

void ModelManager::Render(const Camera &camera) const
{
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
}

void ModelManager::NewModelFromData(Console &console, bool clickNew)
{
    if(!ImGui::BeginPopup("new model from data", ImGuiWindowFlags_AlwaysAutoResize))
        return;

    static char nameBuf[256] = "";
    if(clickNew)
        nameBuf[0] = '\0';

    ImGui::InputText("name", nameBuf, 256);

    ImGui::SameLine();

    if(ImGui::Button("ok"))
    {
        auto data = dataMgr_.GetSelectedMeshGroup();
        if(!data)
        {
            console.AddText(ConsoleText::Error, "No model data is selected");
            ImGui::CloseCurrentPopup();
            return;
        }

        // TODO
        console.AddText(ConsoleText::Normal, "Model created: " + std::string(nameBuf));
        ImGui::CloseCurrentPopup();
    }
}
