#include <iostream>

#include <Atrc/Editor/EditorCore.h>
#include <Atrc/Editor/LauncherScriptExporter.h>
#include <Atrc/Editor/ResourceManagement/EntityCreator.h>
#include <Atrc/Editor/ScreenAxis.h>
#include "SH2DScriptExporter.h"

void EditorCore::Initialize()
{
    AGZ_ASSERT(!data_ && !sState_ && !lState_);

    data_ = std::make_unique<EditorData>();
    sState_ = std::make_unique<WithinFrameState>();
    lState_ = std::make_unique<BetweenFrameState>();

    RegisterResourceCreators(data_->rscMgr);

    data_->filmFilterSlot.SetInstance(data_->rscMgr.Create<FilmFilterInstance>("Box", ""));
    data_->samplerSlot.SetInstance(data_->rscMgr.Create<SamplerInstance>("Native", ""));
    data_->rendererSlot.SetInstance(data_->rscMgr.Create<RendererInstance>("PathTracing", ""));

    lState_->scriptBrowser.SetLabel("script");
    lState_->scriptBrowser.SetTarget(true);
    lState_->scriptBrowser.SetCurrentDirectory("");
    data_->scriptSlot.SetFilename(lState_->scriptBrowser.GetResult());

    lState_->workspaceBrowser.SetLabel("workspace");
    lState_->workspaceBrowser.SetTarget(true);
    lState_->workspaceBrowser.SetCurrentDirectory("");
    data_->workspaceSlot.SetFilename(lState_->workspaceBrowser.GetResult());

    std::strcpy(data_->outputFilenameBuf.data(), "$Output.png");
}

void EditorCore::Destroy()
{
    if(lState_ && lState_->sceneRenderer.IsRendering())
    {
        lState_->sceneRenderer.Stop();
        lState_->sceneRenderer.Clear();
    }

    sState_ = nullptr;
    lState_ = nullptr;
    data_ = nullptr;
}

void EditorCore::ShowMenuMenuBar()
{
    if(!lState_->showMenuBar || !ImGui::BeginMainMenuBar())
        return;
    AGZ::ScopeGuard exitMainMenuBar([] { ImGui::EndMainMenuBar(); });

    if(ImGui::BeginMenu("file"))
    {
        if(ImGui::MenuItem("render"))
        {
            std::string scriptDir = data_->scriptSlot.GetExportedFilename("", "");
            std::string workspaceDir = data_->workspaceSlot.GetExportedFilename("", scriptDir);
            SceneExportingContext ctx(
                &data_->defaultRenderingCamera,
                data_->rscMgr.GetPool<CameraInstance>().GetSelectedInstance().get(),
                data_->filmFilterSlot.GetInstance().get(),
                data_->samplerSlot.GetInstance().get(),
                workspaceDir,
                scriptDir,
                data_->filmSize);
            LauncherScriptExporter exporter(data_->rscMgr, ctx);

            lState_->sceneRendererTex = GL::Texture2D(true);
            lState_->sceneRendererTex.InitializeFormat(1, data_->filmSize.x, data_->filmSize.y, GL_RGB8);

            try
            {
                if(lState_->sceneRenderer.IsRendering())
                {
                    lState_->sceneRenderer.Stop();
                    lState_->sceneRenderer.Clear();
                }

                std::string configStr = exporter.Export(
                    data_->rendererSlot.GetInstance().get(), data_->outputFilenameBuf.data());
                std::cout << configStr;

                AGZ::Config config;
                if(!config.LoadFromMemory(configStr))
                {
                    Global::ShowErrorMessage("failed to load configure content");
                    throw std::runtime_error("");
                }

                if(!lState_->sceneRenderer.Start(config, scriptDir))
                {
                    Global::ShowErrorMessage("sceneRenderer.Start returns false");
                    throw std::runtime_error("");
                }
            }
            catch(...)
            {
                lState_->sceneRenderer.Clear();
                Global::ShowErrorMessage("failed to start rendering");
            }
        }
        if(ImGui::MenuItem("load"))
            sState_->openLoadingWindow = true;
        if(ImGui::MenuItem("save"))
            sState_->openSavingWindow = true;
        if(ImGui::MenuItem("export SH2D"))
            sState_->openExportingSH2DWindow = true;
        ImGui::EndMenu();
    }
    if(ImGui::MenuItem("global"))
        sState_->openGlobalSettingWindow = true;
    if(ImGui::MenuItem("help"))
        sState_->openGlobalHelpWindow = true;
}

void EditorCore::ShowGlobalHelpWindow(const AGZ::Input::Keyboard &kb)
{
    if(sState_->openGlobalHelpWindow)
        ImGui::OpenPopup("global help");
    ImGui::SetNextWindowSize(ImVec2(640, 640), ImGuiCond_FirstUseEver);
    if(!ImGui::BeginPopupModal("global help", nullptr))
        return;
    AGZ::ScopeGuard windowExitGuard([] { ImGui::EndPopup(); });

    static const auto MSG =
        AGZ::Trim(u8R"____(
Hold mouse middle button to adjust camera direction when there is no focused window.
Press ESC to close this help window.
)____");
    ImGui::TextWrapped("%s", MSG.c_str());

    if(kb.IsKeyPressed(AGZ::Input::KEY_ESCAPE))
        ImGui::CloseCurrentPopup();
}

void EditorCore::ShowGlobalSettingWindow(const AGZ::Input::Keyboard &kb)
{
    if(sState_->openGlobalSettingWindow)
    {
        ImGui::OpenPopup("global setting");
        ImGui::SetNextWindowSize(ImVec2(600, 800), ImGuiCond_FirstUseEver);
    }

    if(!ImGui::BeginPopupModal("global setting", nullptr))
        return;
    AGZ::ScopeGuard popupExitGuard([] { ImGui::EndPopup(); });

    ImGui::Text("script directory"); ImGui::SameLine();
    data_->scriptSlot.Display(lState_->scriptBrowser);

    ImGui::Text("workspace"); ImGui::SameLine();
    data_->workspaceSlot.Display(lState_->workspaceBrowser);

    ImGui::InputText("output filename", data_->outputFilenameBuf.data(), data_->outputFilenameBuf.size());

    ImGui::InputInt2("film size", &data_->filmSize[0]);

    if(ImGui::TreeNodeEx("film filter", ImGuiTreeNodeFlags_DefaultOpen))
    {
        data_->filmFilterSlot.Display(data_->rscMgr);
        ImGui::TreePop();
    }

    if(ImGui::TreeNodeEx("sampler", ImGuiTreeNodeFlags_DefaultOpen))
    {
        data_->samplerSlot.Display(data_->rscMgr);
        ImGui::TreePop();
    }

    if(ImGui::TreeNodeEx("renderer", ImGuiTreeNodeFlags_DefaultOpen))
    {
        data_->rendererSlot.Display(data_->rscMgr);
        ImGui::TreePop();
    }

    if(kb.IsKeyPressed(AGZ::Input::KEY_ESCAPE))
        ImGui::CloseCurrentPopup();
}

void EditorCore::ShowExportingSH2DWindow()
{
    static FileBrowser fileBrowser("save to", true, "");
    static std::array<char, 512> filenameBuf;

    ImGui::PushID(&fileBrowser);
    AGZ::ScopeGuard popIDGuard([] { ImGui::PopID(); });

    if(sState_->openExportingSH2DWindow)
    {
        std::strcpy(filenameBuf.data(), "sh2d.txt");
        ImGui::OpenPopup("export to SH2D script");
    }

    if(!ImGui::BeginPopupModal("export to SH2D script", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        return;
    AGZ::ScopeGuard exitPopupGuard([] { ImGui::EndPopup(); });

    static int SHOrder = 4;
    static int workerCount = -1;
    static int taskGirdSize = 32;

    ImGui::InputInt("SH order", &SHOrder);
    ImGui::InputInt("worker count", &workerCount);
    ImGui::InputInt("task grid size", &taskGirdSize);

    if(ImGui::Button("directory"))
        ImGui::OpenPopup(fileBrowser.GetLabel().c_str());
    fileBrowser.Display();
    ImGui::SameLine();
    ImGui::Text("%s", fileBrowser.GetResult().c_str());

    ImGui::InputText("filename", filenameBuf.data(), filenameBuf.size());

    if(ImGui::Button("ok") && filenameBuf[0])
    {
        try
        {
            std::string scriptDir = fileBrowser.GetResult(false);
            std::string workspaceDir = data_->workspaceSlot.GetExportedFilename("", scriptDir);
            SceneExportingContext ctx(
                &data_->defaultRenderingCamera,
                data_->rscMgr.GetPool<CameraInstance>().GetSelectedInstance().get(),
                data_->filmFilterSlot.GetInstance().get(),
                data_->samplerSlot.GetInstance().get(),
                workspaceDir,
                scriptDir,
                data_->filmSize);
            SH2DSceneScriptExporter exporter;

            AGZ::FileSys::WholeFile::WriteText((std::filesystem::path(scriptDir) / filenameBuf.data()).string(),
                exporter.Export(data_->rscMgr, ctx, workerCount, taskGirdSize, SHOrder));
        }
        catch(const std::exception &err)
        {
            Global::ShowErrorMessage(err.what());
        }
        catch(...)
        {
            Global::ShowErrorMessage("an unknown error occurred");
        }
        ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();

    if(ImGui::Button("cancel"))
        ImGui::CloseCurrentPopup();
}

void EditorCore::ShowSavingWindow()
{
    static FileBrowser fileBrowser("save to", true, "");
    static std::array<char, 512> filenameBuf;

    ImGui::PushID(&fileBrowser);
    AGZ::ScopeGuard popIDGuard([] { ImGui::PopID(); });

    if(sState_->openSavingWindow)
    {
        std::strcpy(filenameBuf.data(), "scene.txt");
        ImGui::OpenPopup("saving");
    }

    if(!ImGui::BeginPopupModal("saving", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        return;
    AGZ::ScopeGuard exitPopupGuard([] { ImGui::EndPopup(); });

    if(ImGui::Button("directory"))
        ImGui::OpenPopup(fileBrowser.GetLabel().c_str());
    fileBrowser.Display();
    ImGui::SameLine();
    ImGui::Text("%s", fileBrowser.GetResult().c_str());

    ImGui::InputText("filename", filenameBuf.data(), filenameBuf.size());

    if(ImGui::Button("ok") && filenameBuf[0])
    {
        try
        {
            std::string scriptDir = fileBrowser.GetResult(false);
            std::string workspaceDir = data_->workspaceSlot.GetExportedFilename("", scriptDir);
            SceneExportingContext ctx(
                &data_->defaultRenderingCamera,
                data_->rscMgr.GetPool<CameraInstance>().GetSelectedInstance().get(),
                data_->filmFilterSlot.GetInstance().get(),
                data_->samplerSlot.GetInstance().get(),
                workspaceDir,
                scriptDir,
                data_->filmSize);
            LauncherScriptExporter exporter(data_->rscMgr, ctx);

            AGZ::FileSys::WholeFile::WriteText((std::filesystem::path(scriptDir) / filenameBuf.data()).string(),
                exporter.Export(data_->rendererSlot.GetInstance().get(), data_->outputFilenameBuf.data()));
        }
        catch(const std::exception &err)
        {
            Global::ShowErrorMessage(err.what());
        }
        catch(...)
        {
            Global::ShowErrorMessage("an unknown error occurred");
        }
        ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();

    if(ImGui::Button("cancel"))
        ImGui::CloseCurrentPopup();
}

void EditorCore::ShowLoadingWindow()
{
    ImGui::PushID(&lState_->loadingFilenameBrowser);
    AGZ::ScopeGuard popIDGuard([] { ImGui::PopID(); });

    if(sState_->openLoadingWindow)
        ImGui::OpenPopup(lState_->loadingFilenameBrowser.GetLabel().c_str());

    if(lState_->loadingFilenameBrowser.Display())
    {
        std::string filename = lState_->loadingFilenameBrowser.GetResult();
        try
        {
            AGZ::Config config;
            if(!config.LoadFromFile(filename))
            {
                Global::ShowErrorMessage("failed to load configuration from " + filename);
                return;
            }

            LauncherScriptImporter importer;
            importer.Import(config.Root(), data_.get(), absolute(std::filesystem::path(filename)).parent_path().string());
        }
        catch(const std::exception &err)
        {
            Global::ShowErrorMessage(err.what());
        }
        catch(...)
        {
            Global::ShowErrorMessage("an unknown error occurred");
        }
    }
}

void EditorCore::ShowResourceManager()
{
    ImGui::SetNextWindowPos(ImVec2(sState_->sceneMgrPosX, sState_->sceneMgrPosY), ImGuiCond_FirstUseEver);
    if(ImGui::Begin("scene manager", nullptr, ImVec2(0, 0), -1,
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
    {
        AGZ::ScopeGuard windowExitGuard([] { ImGui::End(); });

        if(ImGui::BeginTabBar("scene manager tab"))
        {
            if(ImGui::BeginTabItem("entity"))
            {
                auto &pool = data_->rscMgr.GetPool<EntityInstance>();
                pool.Display(data_->rscMgr);
                sState_->isEntityPoolDisplayed = true;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("light"))
            {
                auto &pool = data_->rscMgr.GetPool<LightInstance>();
                pool.Display(data_->rscMgr);
                sState_->isLightPoolDisplayed = true;
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("camera"))
            {
                auto &pool = data_->rscMgr.GetPool<CameraInstance>();
                pool.Display(data_->rscMgr);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }

    ImGui::SetNextWindowPos(ImVec2(sState_->sceneMgrPosX, sState_->sceneMgrPosY + 320), ImGuiCond_FirstUseEver);
    if(ImGui::Begin("resource", nullptr, ImVec2(0, 0), -1,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
    {
        AGZ::ScopeGuard windowExitGuard([] { ImGui::End(); });

        if(ImGui::BeginTabBar("object tab"))
        {
            if(ImGui::BeginTabItem("material"))
            {
                auto &pool = data_->rscMgr.GetPool<MaterialInstance>();
                pool.Display(data_->rscMgr);
                if(auto mat = pool.GetSelectedInstance())
                    mat->Display(data_->rscMgr);
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("texture"))
            {
                auto &pool = data_->rscMgr.GetPool<TextureInstance>();
                pool.Display(data_->rscMgr);
                if(auto tex = pool.GetSelectedInstance())
                    tex->Display(data_->rscMgr);
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("geometry"))
            {
                auto &pool = data_->rscMgr.GetPool<GeometryInstance>();
                pool.Display(data_->rscMgr);
                if(auto data = pool.GetSelectedInstance())
                    data->Display(data_->rscMgr);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
}

void EditorCore::ShowCamera()
{
    auto selectedCamera = data_->rscMgr.GetPool<CameraInstance>().GetSelectedInstance();
    ImGui::SetNextWindowPos(ImVec2(sState_->sceneMgrPosX, sState_->sceneMgrPosY + 320 + 320), ImGuiCond_FirstUseEver);
    if(ImGui::Begin("camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        data_->defaultRenderingCamera.Display();
        if(selectedCamera)
            selectedCamera->Display(data_->rscMgr);
        ImGui::End();
    }

    if(selectedCamera)
    {
        float filmAspectRatio = static_cast<float>(data_->filmSize.x) / data_->filmSize.y;
        sState_->selectedCameraProjData = selectedCamera->GetProjData(filmAspectRatio);
    }
}

void EditorCore::ShowEntityEditor()
{
    auto selectedCamera = data_->rscMgr.GetPool<CameraInstance>().GetSelectedInstance();

    if(auto selectedEntity = data_->rscMgr.GetPool<EntityInstance>().GetSelectedInstance(); selectedEntity && sState_->isEntityPoolDisplayed)
    {
        ImGui::SetNextWindowPos(ImVec2(sState_->sceneMgrPosX + 420, sState_->sceneMgrPosY), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("property", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            selectedEntity->DisplayEx(
                data_->rscMgr,
                (selectedCamera ?
                    sState_->selectedCameraProjData.projMatrix : data_->defaultRenderingCamera.GetProjMatrix()),
                data_->defaultRenderingCamera.GetViewMatrix(),
                !lState_->sceneRenderer.IsRendering());
            ImGui::End();
        }
    }
}

void EditorCore::ShowLightEditor()
{
    if(auto selectedLight = data_->rscMgr.GetPool<LightInstance>().GetSelectedInstance(); selectedLight && sState_->isLightPoolDisplayed)
    {
        ImGui::SetNextWindowPos(ImVec2(sState_->sceneMgrPosX + 420, sState_->sceneMgrPosY), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("property", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            selectedLight->Display(data_->rscMgr);
            ImGui::End();
        }
    }
}

void EditorCore::UpdateCamera(const AGZ::Input::Keyboard &kb, const AGZ::Input::Mouse &m)
{
    if(!ImGui::IsAnyWindowFocused())
        data_->defaultRenderingCamera.UpdatePositionAndDirection(kb, m);
    data_->defaultRenderingCamera.UpdateMatrix();
}

void EditorCore::RenderScene()
{
    GL::RenderContext::SetClearColor(Vec4f(Vec3f(0.2f), 0.0f));
    GL::RenderContext::ClearColorAndDepth();

    GL::RenderContext::EnableDepthTest();

    // 场景绘制

    BeginEntityRendering();
    auto selectedCamera = data_->rscMgr.GetPool<CameraInstance>().GetSelectedInstance();
    if(selectedCamera)
    {
        auto VP = sState_->selectedCameraProjData.projMatrix * data_->defaultRenderingCamera.GetViewMatrix();
        for(auto &ent : data_->rscMgr.GetPool<EntityInstance>())
            ent->Render(VP, data_->defaultRenderingCamera.GetPosition());

        SetFullViewport();
    }
    else
    {
        auto defaultCameraProjViewMat = data_->defaultRenderingCamera.GetProjMatrix() * data_->defaultRenderingCamera.GetViewMatrix();
        for(auto &ent : data_->rscMgr.GetPool<EntityInstance>())
            ent->Render(defaultCameraProjViewMat, data_->defaultRenderingCamera.GetPosition());
    }
    EndEntityRendering();
}

void EditorCore::SaveRenderingResult()
{
    lState_->sceneRenderer.CheckSaving();
}

void EditorCore::ShowGUI(GL::Immediate2D &imm, const AGZ::Input::Keyboard &kb)
{
    GL::RenderContext::DisableDepthTest();
    GL::RenderContext::ClearDepth();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glLineWidth(3);

    float fbW = sState_->fbW, fbH = sState_->fbH;

    auto selectedCamera = data_->rscMgr.GetPool<CameraInstance>().GetSelectedInstance();
    if(selectedCamera)
    {
        float LTx = (std::max)(0.0f, 0.5f * fbW - 0.5f * sState_->selectedCameraProjData.viewportWidth);
        float LTy = (std::min)(fbH, 0.5f * fbH - 0.5f *  sState_->selectedCameraProjData.viewportHeight);
        float RBx = (std::min)(fbW, 0.5f * fbW + 0.5f *  sState_->selectedCameraProjData.viewportWidth);
        float RBy = (std::max)(0.0f, 0.5f * fbH + 0.5f * sState_->selectedCameraProjData.viewportHeight);
        imm.DrawQuadP(
            { LTx, LTy }, { RBx, RBy },
            { 1.0f, 1.0f, 1.0f, 0.3f }, false);
    }
    ScreenAxis().Display(
        data_->defaultRenderingCamera.GetProjMatrix() * data_->defaultRenderingCamera.GetViewMatrix(), imm);

    glLineWidth(1);

    // 渲染结果预览

    if(lState_->sceneRenderer.IsRendering())
    {
        if(kb.IsKeyPressed('C'))
        {
            lState_->sceneRenderer.Stop();
            lState_->sceneRenderer.Clear();
            Global::ShowNormalMessage("rendering interrupted");
        }
    }

    if(lState_->sceneRenderer.IsRendering())
    {
        lState_->sceneRenderer.ProcessImage([&](const AGZ::Texture2D<Vec3f> &img)
        {
            lState_->sceneRendererTex.ReinitializeData(img.GetWidth(), img.GetHeight(), img.RawData());
        });
        float LTx = (std::max)(0.0f, 0.5f * fbW - 0.5f * sState_->selectedCameraProjData.viewportWidth);
        float LTy = (std::min)(fbH, 0.5f * fbH - 0.5f *  sState_->selectedCameraProjData.viewportHeight);
        glViewport(
            static_cast<int>(LTx),
            static_cast<int>(LTy),
            static_cast<GLsizei>( sState_->selectedCameraProjData.viewportWidth),
            static_cast<GLsizei>(sState_->selectedCameraProjData.viewportHeight));
        imm.DrawTexturedQuad({ -1, -1 }, { 1, 1 }, lState_->sceneRendererTex);
        SetFullViewport();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glDisable(GL_BLEND);
}
