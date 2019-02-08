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

        void Export(std::stringstream &sst, const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            auto cam = ctx.activeCamera;
            sst << ctx.Indent() << "type = Pinhole;\n";
            sst << ctx.Indent() << "sensorWidth = " << sensorWidth_ << ";\n";
            sst << ctx.Indent() << "sensorDistance = " << sensorWidth_ * cam->GetProjHeight() / cam->GetProjWidth() * Tan(cam->GetProjFOVy() * 0.5f) << ";\n";
            sst << ctx.Indent() << "pos = " << AGZ::To<char>(cam->GetPosition()) << ";\n";
            sst << ctx.Indent() << "lookAt = " << AGZ::To<char>(cam->GetLookAt()) << ";\n";
            sst << ctx.Indent() << "up = (0, 0, 1);\n";
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
