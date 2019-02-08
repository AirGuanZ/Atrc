#include <Atrc/ModelViewer/ResourceManagement/CameraCreator.h>

namespace
{
    class PinholeCameraInstance : public CameraInstance
    {
        float sensorWidth_ = 0.2f;

    public:

        using CameraInstance::CameraInstance;

        void Display(ResourceManager&) override
        {
            ImGui::InputFloat("sensor width", &sensorWidth_);
        }

        void Export(const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            auto cam = ctx.activeCamera;
            ctx.AddLine("type = Pinhole;");
            ctx.AddLine("sensorWidth = ", std::to_string(sensorWidth_), ";");
            ctx.AddLine("sensorDistance = ", 
                std::to_string(sensorWidth_ * cam->GetProjHeight() / cam->GetProjWidth() * Tan(cam->GetProjFOVy() * 0.5f)), ";");
            ctx.AddLine("pos = ", AGZ::To<char>(cam->GetPosition()), ";");
            ctx.AddLine("lookAt = ", AGZ::To<char>(cam->GetLookAt()), ";");
            ctx.AddLine("up = (0, 1, 0);");
        }
    };
}

void RegisterCameraCreators(ResourceManager &rscMgr)
{
    static const PinholeCameraCreator iPinholeCameraCreator;
    rscMgr.AddCreator(&iPinholeCameraCreator);
}

std::shared_ptr<CameraInstance> PinholeCameraCreator::Create(std::string name) const
{
    return std::make_shared<PinholeCameraInstance>(std::move(name));
}
