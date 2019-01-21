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

void ModelManager::Display()
{
    
}
