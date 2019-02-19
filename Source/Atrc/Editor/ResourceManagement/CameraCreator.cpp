#include <Atrc/Editor/ResourceManagement/CameraCreator.h>
#include <Atrc/Editor/Global.h>
#include "Atrc/Mgr/Common.h"
#include "Atrc/Mgr/Parser.h"

namespace
{
    class PinholeCameraInstance : public CameraInstance
    {
        float sensorWidth_  = 0.2f;
        bool autoAspect_    = true;
        float sensorHeight_ = 0.2f; // 只在autoAspect为false的情况下有意义
        Deg FOVy_ = Deg(60);

    protected:

        void Export(const ResourceManager&, LauncherScriptExportingContext &ctx) const override
        {
            auto cam = ctx.activeCamera;
            ctx.AddLine("type = Pinhole;");
            ctx.AddLine("sensorWidth = ", std::to_string(sensorWidth_), ";");
            if(autoAspect_)
            {
                float ratio = static_cast<float>(ctx.outputFilmSize.y) / ctx.outputFilmSize.x * Cot(FOVy_ * 0.5f) * 0.5f;
                ctx.AddLine("sensorDistance = ", sensorWidth_ * ratio, ";");
            }
            else
            {
                ctx.AddLine("sensorHeight = ", sensorHeight_, ";");
                ctx.AddLine("sensorDistance = ",
                    sensorWidth_ * sensorHeight_ / sensorWidth_ * Cot(FOVy_ * 0.5f) * 0.5f, ";");
            }
            ctx.AddLine("pos = ", AGZ::To<char>(cam->GetPosition()), ";");
            ctx.AddLine("lookAt = ", AGZ::To<char>(cam->GetLookAt()), ";");
            ctx.AddLine("up = (0, 1, 0);");
        }

    public:
        
        using CameraInstance::CameraInstance;

        void Display(ResourceManager&) override
        {
            ImGui::Checkbox("auto aspect", &autoAspect_);
            ImGui::InputFloat("sensor width", &sensorWidth_);
            if(!autoAspect_)
                ImGui::InputFloat("sensor height", &sensorHeight_);
            ImGui::InputFloat("FOV", &FOVy_.value);
        }

        ProjData GetProjData(float dstAspectRatio) const override
        {
            float fbW = static_cast<float>(Global::GetFramebufferWidth());
            float fbH = static_cast<float>(Global::GetFramebufferHeight());
            float fbAspectRatio = fbW / fbH;

            if(!autoAspect_)
                dstAspectRatio = sensorWidth_ / sensorHeight_;

            ProjData ret;

            if(dstAspectRatio > fbAspectRatio)
            {
                ret.viewportWidth = fbW - 100;
                ret.viewportHeight = ret.viewportWidth / dstAspectRatio;
            }
            else
            {
                ret.viewportHeight = fbH - 100;
                ret.viewportWidth = ret.viewportHeight * dstAspectRatio;
            }

            Deg alpha = Rad2Deg(Rad(2 * AGZ::Math::Arctan(fbH / ret.viewportHeight * AGZ::Math::Tan(FOVy_ * 0.5f))));
            ret.projMatrix = Mat4f::Perspective(alpha, fbAspectRatio, 0.1f, 1000.0f);

            return ret;
        }

        void Import(const ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params) override
        {
            sensorWidth_ = std::stof(params["sensorWidth"].AsValue());

            if(auto nSensorHeight = params.Find("sensorHeight"))
            {
                autoAspect_ = false;
                sensorHeight_ = std::stof(nSensorHeight->AsValue());
            }
            else
                autoAspect_ = true;

            float sensorDistance = std::stof(params["sensorDistance"].AsValue());

            if(autoAspect_)
            {
                Vec2i filmSize = Atrc::Mgr::Parser::ParseVec2i(root["film.size"]);
                float autoSensorHeight = sensorWidth_ * filmSize.y / filmSize.x;
                FOVy_ = Deg(Rad(2 * AGZ::Math::Arctan(autoSensorHeight / 2 / sensorDistance)));
            }
            else
                FOVy_ = Deg(Rad(2 * AGZ::Math::Arctan(sensorHeight_ / 2 / sensorDistance)));
        }
    };
}

void RegisterCameraCreators(ResourceManager &rscMgr)
{
    static const PinholeCameraCreator iPinholeCameraCreator;
    rscMgr.AddCreator(&iPinholeCameraCreator);
}

std::shared_ptr<CameraInstance> PinholeCameraCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<PinholeCameraInstance>(GetName(), std::move(name));
}
