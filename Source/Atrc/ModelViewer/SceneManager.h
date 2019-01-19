#pragma once

#include <unordered_map>

#include "ModelDataManager.h"

class Model;

class SceneManager
{
public:

    void CreateModel(AGZ::Str8 name, const ModelDataManager::MeshGroupData *data);

private:

    std::unordered_map<AGZ::Str8, Model*> name2Model_;
};
