#pragma once

#include <memory>

#include <Atrc/Editor/Camera.h>
#include <Atrc/Editor/FilenameSlot.h>
#include <Atrc/Editor/GL.h>
#include <Atrc/Editor/Global.h>
#include <Atrc/Editor/ResourceManagement/ResourceManager.h>
#include <Atrc/Editor/SceneRenderer.h>

struct EditorData
{
    DefaultRenderingCamera defaultRenderingCamera;

    ResourceManager rscMgr;

    Vec2i filmSize = { 640, 480 };
    FilmFilterSlot filmFilterSlot;
    SamplerSlot    samplerSlot;
    RendererSlot   rendererSlot;
    TFilenameSlot<false, FilenameMode::Absolute> scriptSlot;
    TFilenameSlot<false, FilenameMode::RelativeToScript> workspaceSlot;
    std::array<char, 512> outputFilenameBuf = { '\0' };
};

class EditorCore
{
    struct WithinFrameState
    {
        bool openGlobalHelpWindow    = false;
        bool openGlobalSettingWindow = false;
        bool openExportingSH2DWindow = false;
        bool openSavingWindow        = false;
        bool openLoadingWindow       = false;

        float fbW = 1, fbH = 1;
        float sceneMgrPosX = 0, sceneMgrPosY = 0;

        bool isEntityPoolDisplayed = false;
        bool isLightPoolDisplayed = false;

        CameraInstance::ProjData selectedCameraProjData;
    };

    struct BetweenFrameState
    {
        FileBrowser scriptBrowser;
        FileBrowser workspaceBrowser;

        bool showMenuBar = true;

        SceneRenderer sceneRenderer;
        GL::Texture2D sceneRendererTex;

        FileBrowser loadingFilenameBrowser = FileBrowser("load from", false, "");
    };

    std::unique_ptr<EditorData> data_;
    std::unique_ptr<WithinFrameState> sState_;
    std::unique_ptr<BetweenFrameState> lState_;

public:

    void Initialize();

    void Destroy();

    void ShowMenuMenuBar();

    void ShowGlobalHelpWindow(const AGZ::Input::Keyboard &kb);

    void ShowGlobalSettingWindow(const AGZ::Input::Keyboard &kb);

    void ShowExportingSH2DWindow();

    void ShowSavingWindow();

    void ShowLoadingWindow();

    void ShowResourceManager();

    void ShowCamera();

    void ShowEntityEditor();

    void ShowLightEditor();

    void UpdateCamera(const AGZ::Input::Keyboard &kb, const AGZ::Input::Mouse &m);

    void RenderScene();

    void SaveRenderingResult();

    void ShowGUI(GL::Immediate2D &imm, const AGZ::Input::Keyboard &kb);

    void OnFramebufferResized()
    {
        data_->defaultRenderingCamera.AutoResizeProj();
    }

    void OnKeyDown(AGZ::Input::Key k)
    {
        if(k == AGZ::Input::KEY_F2 && lState_)
            lState_->showMenuBar = !lState_->showMenuBar;
    }

    void BeginFrame()
    {
        *sState_ = WithinFrameState();

        sState_->fbW = static_cast<float>(Global::GetFramebufferWidth());
        sState_->fbH = static_cast<float>(Global::GetFramebufferHeight());
        sState_->sceneMgrPosX = 40;
        sState_->sceneMgrPosY = ImGui::GetFrameHeight() + sState_->sceneMgrPosX * (sState_->fbH / sState_->fbW);
    }

    void EndFrame()
    {
        
    }

    void SetFullViewport()
    {
        glViewport(0, 0, Global::GetFramebufferWidth(), Global::GetFramebufferHeight());
    }
};
