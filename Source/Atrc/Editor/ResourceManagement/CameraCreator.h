#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>

void RegisterCameraCreators(ResourceManager &rscMgr);

class PinholeCameraCreator : public CameraCreator
{
public:

    PinholeCameraCreator() : CameraCreator("pinhole") { }

    std::shared_ptr<CameraInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};
