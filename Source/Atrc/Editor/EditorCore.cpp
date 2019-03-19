#include <iostream>

#include <Atrc/Editor/EditorCore.h>
#include <Atrc/Editor/LauncherScriptExporter.h>
#include <Atrc/Editor/ResourceManagement/EntityCreator.h>
#include <Atrc/Editor/SH2DScriptExporter.h>
#include <Atrc/Editor/SH2DLightScriptExporter.h>
#include <Lib/imgui/imgui/ImGuizmo.h>

void EditorCore::UpdateRenderPvRect()
{
    if(data_->rscMgr.GetPool<CameraInstance>().GetSelectedInstance())
    {
        float pvW = Global::PvWf(), pvH = Global::PvHf();
        lState_->renderPvLx = Global::PvX() + (std::max)(0.0f, 0.5f * pvW - 0.5f * lState_->selectedCameraProjData.viewportWidth);
        lState_->renderPvBy = Global::FbH() - (Global::PvY() + (std::min)(pvH, 0.5f * pvH - 0.5f *  lState_->selectedCameraProjData.viewportHeight));
        lState_->renderPvRx = Global::PvX() + (std::min)(pvW, 0.5f * pvW + 0.5f *  lState_->selectedCameraProjData.viewportWidth);
        lState_->renderPvTy = Global::FbH() - (Global::PvY() + (std::max)(0.0f, 0.5f * pvH + 0.5f * lState_->selectedCameraProjData.viewportHeight));
    }
}

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

    data_->scriptFilename.SetFilename(std::filesystem::current_path() / "scene.txt");
    data_->workspaceDir.SetFilename(std::filesystem::current_path());

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
            try
            {
                auto scriptDir = absolute(data_->scriptFilename.GetFilename()).parent_path();
                auto workspaceDir = absolute(data_->workspaceDir.GetFilename());
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

                if(!lState_->sceneRenderer.Start(config, scriptDir.string()))
                {
                    Global::ShowErrorMessage("sceneRenderer.Start returns false");
                    throw std::runtime_error("");
                }

                UpdateRenderPvRect();
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
        if(ImGui::MenuItem("export SH2D scene"))
            sState_->openExportingSH2DWindow = true;
        if(ImGui::MenuItem("export SH2D light"))
            sState_->openExportingLightWindow = true;
        ImGui::EndMenu();
    }
    if(ImGui::MenuItem("global"))
        sState_->openGlobalSettingWindow = true;
    if(ImGui::MenuItem("help"))
        sState_->openGlobalHelpWindow = true;

    sState_->mainMenuBarHeight = ImGui::GetWindowHeight();
}

bool EditorCore::BeginLeftWindow()
{
    const float sizeY = Global::FbHf() - sState_->mainMenuBarHeight;
    ImGui::SetNextWindowPos(ImVec2(0, sState_->mainMenuBarHeight));
    ImGui::SetNextWindowSizeConstraints(
        ImVec2(100, sizeY),
        ImVec2(Global::FbWf() / 2 - 100, sizeY));
    return ImGui::Begin("left window", nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResizeGrip | ImGuiWindowFlags_NoTitleBar);
}

void EditorCore::EndLeftWindow()
{
    sState_->leftWindowSizeX = ImGui::GetWindowWidth();
    ImGui::End();
}

bool EditorCore::BeginRightWindow()
{
    const float sizeY = Global::FbHf() - sState_->mainMenuBarHeight;
    ImGui::SetNextWindowSizeConstraints(
        ImVec2(100, sizeY),
        ImVec2(Global::FbWf() / 2 - 100, sizeY));
    ImGui::SetNextWindowPos(
        ImVec2(Global::FbWf() - lState_->rightWindowSizeX,
            sState_->mainMenuBarHeight));
    return ImGui::Begin("right window", nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResizeGrip | ImGuiWindowFlags_NoTitleBar);
}

void EditorCore::EndRightWindow()
{
    lState_->rightWindowSizeX = ImGui::GetWindowWidth();
    ImGui::End();
}

bool EditorCore::BeginBottomWindow()
{
    const float sizeX = Global::FbWf() - sState_->leftWindowSizeX - lState_->rightWindowSizeX;
    ImGui::SetNextWindowSizeConstraints(
        ImVec2(sizeX, 100),
        ImVec2(sizeX, Global::FbHf() - 200));
    ImGui::SetNextWindowPos(
        ImVec2(sState_->leftWindowSizeX, Global::FbH() - lState_->bottomWindowSizeY));
    return ImGui::Begin("bottom window", nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResizeGrip | ImGuiWindowFlags_NoTitleBar);
}

void EditorCore::EndBottomWindow()
{
    lState_->bottomWindowSizeY = ImGui::GetWindowHeight();
    ImGui::End();
}

float EditorCore::GetViewportX() const noexcept
{
    return sState_->leftWindowSizeX;
}

float EditorCore::GetViewportY() const noexcept
{
    return lState_->bottomWindowSizeY;
}

float EditorCore::GetViewportWidth() const noexcept
{
    return Global::FbWf() - sState_->leftWindowSizeX - lState_->rightWindowSizeX;
}

float EditorCore::GetViewportHeight() const noexcept
{
    return Global::FbHf() - sState_->mainMenuBarHeight - lState_->bottomWindowSizeY;
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

    if(kb.IsKeyPressed(AGZ::Input::KEY_ESCAPE) && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
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

    ImGui::Text("script filename"); ImGui::SameLine();
    data_->scriptFilename.Display();

    ImGui::Text("workspace"); ImGui::SameLine();
    data_->workspaceDir.Display();

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

    if(kb.IsKeyPressed(AGZ::Input::KEY_ESCAPE) && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
        ImGui::CloseCurrentPopup();
}

void EditorCore::ShowExportingSH2DSceneWindow()
{
    static FileSelector filename(ImGuiFileBrowserFlags_NoModal | ImGuiFileBrowserFlags_EnterNewFilename);

    ImGui::PushID(&filename);
    AGZ::ScopeGuard popIDGuard([] { ImGui::PopID(); });

    if(sState_->openExportingSH2DWindow)
        ImGui::OpenPopup("export to SH2D scene script");

    if(!ImGui::BeginPopupModal("export to SH2D scene script", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        return;
    AGZ::ScopeGuard exitPopupGuard([] { ImGui::EndPopup(); });

    static int SHOrder = 4;
    static int workerCount = -1;
    static int taskGirdSize = 32;

    ImGui::InputInt("SH order", &SHOrder);
    ImGui::InputInt("worker count", &workerCount);
    ImGui::InputInt("task grid size", &taskGirdSize);

    filename.Display();

    if(ImGui::Button("ok") && filename.HasFilename())
    {
        try
        {
            auto scriptDir = absolute(filename.GetFilename()).parent_path();
            auto workspaceDir = absolute(data_->workspaceDir.GetFilename());
            SceneExportingContext ctx(
                &data_->defaultRenderingCamera,
                data_->rscMgr.GetPool<CameraInstance>().GetSelectedInstance().get(),
                data_->filmFilterSlot.GetInstance().get(),
                data_->samplerSlot.GetInstance().get(),
                workspaceDir,
                scriptDir,
                data_->filmSize);
            SH2DSceneScriptExporter exporter;

            AGZ::FileSys::WholeFile::WriteText(filename.GetFilename().string(),
                exporter.Export(data_->rscMgr, ctx, workerCount, taskGirdSize, SHOrder));
            filename.Clear();
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

void EditorCore::ShowExportingSH2DLightWindow()
{
    static FileSelector filename(ImGuiFileBrowserFlags_NoModal | ImGuiFileBrowserFlags_EnterNewFilename);

    ImGui::PushID(&filename);
    AGZ::ScopeGuard popIDGuard([] { ImGui::PopID(); });

    if(sState_->openExportingLightWindow)
        ImGui::OpenPopup("export to SH2D light script");

    if(!ImGui::BeginPopupModal("export to SH2D light script", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        return;
    AGZ::ScopeGuard exitPopupGuard([] { ImGui::EndPopup(); });

    static int SHOrder = 4;
    static int N = 100000;

    ImGui::InputInt("SH order", &SHOrder);
    ImGui::InputInt("N", &N);

    filename.Display();

    if(ImGui::Button("ok") && filename.HasFilename())
    {
        try
        {
            auto scriptDir = absolute(filename.GetFilename()).parent_path();
            auto workspaceDir = absolute(data_->workspaceDir.GetFilename());
            SceneExportingContext ctx(
                &data_->defaultRenderingCamera,
                data_->rscMgr.GetPool<CameraInstance>().GetSelectedInstance().get(),
                data_->filmFilterSlot.GetInstance().get(),
                data_->samplerSlot.GetInstance().get(),
                workspaceDir,
                scriptDir,
                data_->filmSize);
            SH2DLightScriptExporter exporter;

            AGZ::FileSys::WholeFile::WriteText(filename.GetFilename().string(),
                exporter.Export(data_->rscMgr, ctx, SHOrder, N));
            filename.Clear();
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
    static ImGui::FileBrowser filename(ImGuiFileBrowserFlags_NoModal | ImGuiFileBrowserFlags_EnterNewFilename);

    ImGui::PushID(&filename);
    AGZ::ScopeGuard popIDGuard([] { ImGui::PopID(); });

    if(sState_->openSavingWindow)
        filename.Open();

    filename.Display();

    if(filename.HasSelected())
    {
        try
        {
            auto scriptDir = absolute(filename.GetSelected()).parent_path();
            auto workspaceDir = absolute(data_->workspaceDir.GetFilename());
            SceneExportingContext ctx(
                &data_->defaultRenderingCamera,
                data_->rscMgr.GetPool<CameraInstance>().GetSelectedInstance().get(),
                data_->filmFilterSlot.GetInstance().get(),
                data_->samplerSlot.GetInstance().get(),
                workspaceDir,
                scriptDir,
                data_->filmSize);
            LauncherScriptExporter exporter(data_->rscMgr, ctx);

            AGZ::FileSys::WholeFile::WriteText(filename.GetSelected().string(),
                exporter.Export(data_->rendererSlot.GetInstance().get(), data_->outputFilenameBuf.data()));
            filename.ClearSelected();
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

void EditorCore::ShowLoadingWindow()
{
    ImGui::PushID(&lState_->loadingFilename);
    AGZ::ScopeGuard popIDGuard([] { ImGui::PopID(); });

    if(sState_->openLoadingWindow)
    {
        lState_->loadingFilename.SetTitle("load from");
        lState_->loadingFilename.Open();
    }

    lState_->loadingFilename.Display();
    if(lState_->loadingFilename.HasSelected())
    {
        std::string filename = lState_->loadingFilename.GetSelected().string();
        lState_->loadingFilename.ClearSelected();
        try
        {
            AGZ::Config config;
            if(!config.LoadFromFile(filename))
            {
                Global::ShowErrorMessage("failed to load configuration from " + filename);
                return;
            }

            LauncherScriptImporter importer;
            importer.Import(config.Root(), data_.get(), absolute(std::filesystem::path(filename)).string());
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
    if(ImGui::BeginTabBar("scene manager tab", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_Reorderable))
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

    if(ImGui::BeginTabBar("resource manager tab", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_Reorderable))
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

void EditorCore::ShowCamera()
{
    auto selectedCamera = data_->rscMgr.GetPool<CameraInstance>().GetSelectedInstance();

    data_->defaultRenderingCamera.Display();
    if(selectedCamera)
        selectedCamera->Display(data_->rscMgr);
}

void EditorCore::ShowEntityEditor()
{
    auto selectedCamera = data_->rscMgr.GetPool<CameraInstance>().GetSelectedInstance();

    if(auto selectedEntity = data_->rscMgr.GetPool<EntityInstance>().GetSelectedInstance())
    {
        Mat4f projMat = selectedCamera ? lState_->selectedCameraProjData.projMatrix : data_->defaultRenderingCamera.GetProjMatrix();
        ImGuizmo::SetRect(Global::PvXf(), Global::FbHf() - Global::PvYf() - Global::PvHf(), Global::PvWf(), Global::PvHf());
        selectedEntity->DisplayEx(
            data_->rscMgr,
            projMat,
            data_->defaultRenderingCamera.GetViewMatrix(),
            !lState_->sceneRenderer.IsRendering());
    }
}

void EditorCore::ShowLightEditor()
{
    if(auto selectedLight = data_->rscMgr.GetPool<LightInstance>().GetSelectedInstance())
    {
        selectedLight->Display(data_->rscMgr);
    }
}

void EditorCore::UpdateCamera(const AGZ::Input::Keyboard &kb, const AGZ::Input::Mouse &m)
{
    if(!ImGui::IsAnyWindowFocused())
        data_->defaultRenderingCamera.UpdatePositionAndDirection(kb, m);
    data_->defaultRenderingCamera.UpdateMatrix();

    if(auto selectedCamera = data_->rscMgr.GetPool<CameraInstance>().GetSelectedInstance())
    {
        float filmAspectRatio = static_cast<float>(data_->filmSize.x) / data_->filmSize.y;
        lState_->selectedCameraProjData = selectedCamera->GetProjData(filmAspectRatio);
    }
}

void EditorCore::RenderScene()
{
    GL::RenderContext::SetClearColor(Vec4f(Vec3f(0.2f), 0.0f));
    GL::RenderContext::ClearColorAndDepth();

    GL::RenderContext::EnableDepthTest();

    // 场景绘制

    glViewport(Global::PvX(), Global::PvY(), Global::PvW(), Global::PvH());

    BeginEntityRendering();
    auto selectedCamera = data_->rscMgr.GetPool<CameraInstance>().GetSelectedInstance();
    if(selectedCamera)
    {
        auto VP = lState_->selectedCameraProjData.projMatrix * data_->defaultRenderingCamera.GetViewMatrix();
        for(auto &ent : data_->rscMgr.GetPool<EntityInstance>())
            ent->Render(VP, data_->defaultRenderingCamera.GetPosition());
    }
    else
    {
        auto defaultCameraProjViewMat = data_->defaultRenderingCamera.GetProjMatrix() * data_->defaultRenderingCamera.GetViewMatrix();
        for(auto &ent : data_->rscMgr.GetPool<EntityInstance>())
            ent->Render(defaultCameraProjViewMat, data_->defaultRenderingCamera.GetPosition());
    }
    EndEntityRendering();

    SetFullViewport();
}

void EditorCore::SaveRenderingResult()
{
    lState_->sceneRenderer.CheckSaving();
}

void EditorCore::ShowPreviewGUI(GL::Immediate2D &imm, const AGZ::Input::Keyboard &kb)
{
    GL::RenderContext::DisableDepthTest();
    GL::RenderContext::ClearDepth();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SetFullViewport();

    glLineWidth(3);

    auto selectedCamera = data_->rscMgr.GetPool<CameraInstance>().GetSelectedInstance();

    if(selectedCamera)
    {
        float pvW = Global::PvWf(), pvH = Global::PvHf();
        float Lx = Global::PvX() + (std::max)(0.0f, 0.5f * pvW - 0.5f * lState_->selectedCameraProjData.viewportWidth);
        float By = Global::FbH() - (Global::PvY() + (std::min)(pvH, 0.5f * pvH - 0.5f *  lState_->selectedCameraProjData.viewportHeight));
        float Rx = Global::PvX() + (std::min)(pvW, 0.5f * pvW + 0.5f *  lState_->selectedCameraProjData.viewportWidth);
        float Ty = Global::FbH() - (Global::PvY() + (std::max)(0.0f, 0.5f * pvH + 0.5f * lState_->selectedCameraProjData.viewportHeight));
        imm.DrawQuadP(
            { Lx, Ty }, { Rx, By },
            { 1.0f, 1.0f, 1.0f, 0.3f }, false);
    }

    glLineWidth(1);

    if(lState_->sceneRenderer.IsRendering())
    {
        if(kb.IsKeyPressed('C'))
        {
            lState_->sceneRenderer.Stop();
            lState_->sceneRenderer.Clear();
            Global::ShowNormalMessage("rendering interrupted");
        }

        lState_->sceneRenderer.ProcessImage([&](const AGZ::Texture2D<Vec3f> &img)
        {
            lState_->sceneRendererTex.ReinitializeData(img.GetWidth(), img.GetHeight(), img.RawData());
        });
        imm.DrawTexturedQuadP(
            { lState_->renderPvLx, lState_->renderPvBy },
            { lState_->renderPvRx, lState_->renderPvTy }, lState_->sceneRendererTex);
    }

    SetFullViewport();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glDisable(GL_BLEND);
}
