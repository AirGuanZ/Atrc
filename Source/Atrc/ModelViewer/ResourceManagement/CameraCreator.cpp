#include <Atrc/ModelViewer/ResourceManagement/CameraCreator.h>

namespace
{
    class PinholeCameraInstance : public CameraInstance
    {
        float sensorWidth_  = 0.2f;
        bool autoAspect_    = false;
        float sensorHeight_ = 0.2f; // 只在autoAspect为false的情况下有意义
        Deg FOVy_ = Deg(60);

    public:
        
        using CameraInstance::CameraInstance;

        void Display(ResourceManager&) override
        {
            ImGui::Checkbox("auto aspect", &autoAspect_);
            ImGui::InputFloat("sensor width", &sensorWidth_);
            if(autoAspect_)
                ImGui::InputFloat("sensor height", &sensorHeight_);
            ImGui::InputFloat("FOV", &FOVy_.value);
        }

        void Export(const ResourceManager&, LauncherScriptExportingContext &ctx) const override
        {
            auto cam = ctx.activeCamera;
            ctx.AddLine("type = Pinhole;");
            ctx.AddLine("sensorWidth = ", std::to_string(sensorWidth_), ";");
            if(autoAspect_)
            {
                ctx.AddLine("sensorDistance = ",
                    std::to_string(sensorWidth_ * cam->GetProjHeight() / cam->GetProjWidth() * Tan(cam->GetProjFOVy() * 0.5f)), ";");
            }
            else
            {
                ctx.AddLine("sensorHeight = ", sensorHeight_, ";");
                ctx.AddLine("sensorDistance = ",
                    std::to_string(sensorWidth_ * sensorHeight_ / sensorWidth_ * Tan(cam->GetProjFOVy() * 0.5f)), ";");
            }
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
