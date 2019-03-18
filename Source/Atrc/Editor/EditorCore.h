#pragma once

#include <memory>

#include <Atrc/Editor/Camera.h>
#include <Atrc/Editor/FileSelector.h>
#include <Atrc/Editor/GL.h>
#include <Atrc/Editor/Global.h>
#include <Atrc/Editor/ResourceManagement/ResourceManager.h>
#include <Atrc/Editor/SceneRenderer.h>

struct EditorData
{
    DefaultRenderingCamera defaultRenderingCamera;

    ResourceManager rscMgr;

    Vec2i filmSize;
    FilmFilterSlot filmFilterSlot;
    SamplerSlot    samplerSlot;
    RendererSlot   rendererSlot;

    FileSelector scriptFilename;
    FileSelector workspaceDir;

    std::array<char, 512> outputFilenameBuf;

    EditorData()
        : filmSize(640, 480),
          scriptFilename(ImGuiFileBrowserFlags_NoModal | ImGuiFileBrowserFlags_EnterNewFilename),
          workspaceDir(ImGuiFileBrowserFlags_NoModal | ImGuiFileBrowserFlags_SelectDirectory),
          outputFilenameBuf{ '\0' }
    {
        
    }
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

        float mainMenuBarHeight = 0;
        float leftWindowSizeX = 0;
    };

    struct BetweenFrameState
    {
        bool showMenuBar = true;

        SceneRenderer sceneRenderer;
        GL::Texture2D sceneRendererTex;

        float rightWindowSizeX = 0;
        float bottomWindowSizeY = 0;

        ImGui::FileBrowser loadingFilename = ImGui::FileBrowser(ImGuiFileBrowserFlags_NoModal);

        CameraInstance::ProjData selectedCameraProjData;
        float renderPvLx = 0;
        float renderPvRx = 0;
        float renderPvBy = 0;
        float renderPvTy = 0;
    };

    std::unique_ptr<EditorData> data_;
    std::unique_ptr<WithinFrameState> sState_;
    std::unique_ptr<BetweenFrameState> lState_;

    void UpdateRenderPvRect();

public:

    void Initialize();

    void Destroy();

    void ShowMenuMenuBar();

    bool BeginLeftWindow();
    void EndLeftWindow();

    bool BeginRightWindow();
    void EndRightWindow();

    bool BeginBottomWindow();
    void EndBottomWindow();

    float GetViewportX() const noexcept;
    float GetViewportY() const noexcept;
    float GetViewportWidth() const noexcept;
    float GetViewportHeight() const noexcept;

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

    void ShowPreviewGUI(GL::Immediate2D &imm, const AGZ::Input::Keyboard &kb);

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

        sState_->fbW = static_cast<float>(Global::FbW());
        sState_->fbH = static_cast<float>(Global::FbH());
        sState_->sceneMgrPosX = 40;
        sState_->sceneMgrPosY = ImGui::GetFrameHeight() + sState_->sceneMgrPosX * (sState_->fbH / sState_->fbW);
    }

    void EndFrame()
    {
        
    }

    void SetFullViewport()
    {
        glViewport(0, 0, Global::FbW(), Global::FbH());
    }
};
