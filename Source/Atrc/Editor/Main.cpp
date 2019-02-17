#include <iostream>

#include <AGZUtils/Utils/Mesh.h>
#include <AGZUtils/Utils/Texture.h>

#include <Atrc/Editor/Camera.h>
#include <Atrc/Editor/Console.h>
#include <Atrc/Editor/FilenameSlot.h>
#include <Atrc/Editor/GL.h>
#include <Atrc/Editor/Global.h>
#include <Atrc/Editor/LauncherScriptExporter.h>
#include <Atrc/Editor/ResourceManagement/EntityCreator.h>
#include <Atrc/Editor/ResourceManagement/ResourceManager.h>
#include <Atrc/Editor/SceneRenderer.h>
#include <Atrc/Editor/ScreenAxis.h>
#include <Atrc/Editor/EntityController.h>

using namespace std;

constexpr int INIT_WIN_WIDTH = 1400;
constexpr int INIT_WIN_HEIGHT = 900;

void ShowGlobalHelpWindow(bool open, const AGZ::Input::Keyboard &kb)
{
    if(open)
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

void SetFullViewport()
{
    glViewport(0, 0, Global::GetFramebufferWidth(), Global::GetFramebufferHeight());
}

struct EditorData
{
    DefaultRenderingCamera defaultRenderingCamera;

    ResourceManager rscMgr;

    Vec2i filmSize = { 640, 480 };
    FilmFilterSlot filmFilterSlot;
    SamplerSlot samplerSlot;
    RendererSlot rendererSlot;

    TFilenameSlot<false, FilenameMode::Absolute> scriptSlot;
    TFilenameSlot<false, FilenameMode::RelativeToScript> workspaceSlot;
    std::array<char, 512> outputFilenameBuf;
};

int Run(GLFWwindow *window)
{
    using namespace AGZ::GraphicsAPI;
    using namespace AGZ::Input;

    AGZ::ObjArena<> arena;
    
    // 初始化IMGUI

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window);
    ImGui_ImplOpenGL3_Init();

    ImGui::StyleColorsLight();

    ImGui::GetStyle().PopupRounding   = 7;
    ImGui::GetStyle().WindowRounding  = 7;
    ImGui::GetStyle().GrabRounding    = 7;
    ImGui::GetStyle().FrameRounding   = 7;
    ImGui::GetStyle().FrameBorderSize = 1;

    ImFontConfig defaultFontConfig;
    defaultFontConfig.SizePixels = 16.0f;
    ImGui::GetIO().Fonts->AddFontDefault(&defaultFontConfig);

    // 准备输入category

    KeyboardManager<GLFWKeyboardCapturer> keyboardMgr;
    keyboardMgr.GetCapturer().Initialize(window);
    auto &keyboard = keyboardMgr.GetKeyboard();

    MouseManager<GLFWMouseCapturer> mouseMgr;
    mouseMgr.GetCapturer().Initialize(window);
    auto &mouse = mouseMgr.GetMouse();

    WindowManager<GLFWWindowCapturer> winMgr;
    winMgr.GetCapturer().Initialize(window);
    auto &win = winMgr.GetWindow();

    // 注册键盘事件

    keyboard.AttachHandler(arena.Create<KeyDownHandler>(
        [&](const KeyDown &param)
    {
        ImGui_ImplGlfw_KeyDown(param.key);
    }));
    keyboard.AttachHandler(arena.Create<KeyUpHandler>(
        [&](const KeyUp &param)
    {
        ImGui_ImplGlfw_KeyUp(param.key);
    }));
    keyboard.AttachHandler(arena.Create<CharEnterHandler>(
        [&](const CharEnter &param)
    {
        ImGui_ImplGlfw_Char(param.ch);
    }));

    // 注册鼠标事件

    mouse.AttachHandler(arena.Create<MouseButtonDownHandler>(
        [&](const MouseButtonDown &param)
    {
        ImGui_ImplGlfw_MouseButtonDown(param.button);
    }));
    mouse.AttachHandler(arena.Create<WheelScrollHandler>(
        [&](const WheelScroll &param)
    {
        ImGui_ImplGlfw_WheelScroll(param.offset);
    }));

    // Immediate Painter

    GL::Immediate2D imm;
    imm.Initialize({
        static_cast<float>(Global::GetFramebufferWidth()),
        static_cast<float>(Global::GetFramebufferHeight())
    });

    EditorData editorData;

    // 注册窗口事件

    win.AttachHandler(arena.Create<FramebufferSizeHandler>(
        [&](const FramebufferSize &param)
    {
        Global::_setFramebufferWidth(param.w);
        Global::_setFramebufferHeight(param.h);
        editorData.defaultRenderingCamera.AutoResizeProj();
        SetFullViewport();
        imm.Resize({ static_cast<float>(param.w), static_cast<float>(param.h) });
    }));
    
    // Model Manager

	Console console;
    Global::_setConsole(&console);

    // Object Manager

    RegisterResourceCreators(editorData.rscMgr);

    // global setting

    SceneRenderer sceneRenderer;
    GL::Texture2D sceneRendererTex;

    auto initSceneRendererTex = [&](int w, int h)
    {
        sceneRendererTex = GL::Texture2D(true);
        sceneRendererTex.InitializeHandle();
        sceneRendererTex.InitializeFormat(1, w, h, GL_RGB8);
    };

    editorData.filmFilterSlot.SetInstance(editorData.rscMgr.Create<FilmFilterInstance>("box", ""));
    editorData.samplerSlot.SetInstance(editorData.rscMgr.Create<SamplerInstance>("native", ""));
    editorData.rendererSlot.SetInstance(editorData.rscMgr.Create<RendererInstance>("path tracing", ""));

    FileBrowser scriptBrowser("script", true, "");
    FileBrowser workspaceBrowser("workspace", true, "");
    std::strcpy(editorData.outputFilenameBuf.data(), "$Output.png");

    bool showMenuBar = true;
    keyboard.AttachHandler(arena.Create<KeyDownHandler>([&](const KeyDown &param)
    {
        if(param.key == KEY_F2)
            showMenuBar = !showMenuBar;
    }));

    while(!glfwWindowShouldClose(window))
    {
        // 各种事件捕获与传递

        glfwPollEvents();
        keyboardMgr.Capture();
        mouseMgr.Capture();
        winMgr.Capture();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        EntityControllerAction::BeginFrame();

        // 全局菜单栏

        bool openGlobalHelpWindow = false;
        bool openGlobalSettingWindow = false;

        AGZ::Config config;

        if(showMenuBar && ImGui::BeginMainMenuBar())
        {
            if(ImGui::BeginMenu("file"))
            {
                if(ImGui::MenuItem("render"))
                {
                    std::string scriptDir = editorData.scriptSlot.GetExportedFilename("", "");
                    std::string workspaceDir = editorData.workspaceSlot.GetExportedFilename("", scriptDir);
                    LauncherScriptExportingContext ctx(
                        &editorData.defaultRenderingCamera,
                        editorData.rscMgr.GetPool<CameraInstance>().GetSelectedInstance().get(),
                        editorData.filmFilterSlot.GetInstance().get(),
                        editorData.rendererSlot.GetInstance().get(),
                        editorData.samplerSlot.GetInstance().get(),
                        workspaceDir,
                        scriptDir,
                        editorData.outputFilenameBuf.data(),
                        editorData.filmSize);
                    LauncherScriptExporter exporter(editorData.rscMgr, ctx);
                    
                    initSceneRendererTex(editorData.filmSize.x, editorData.filmSize.y);

                    try
                    {
                        if(sceneRenderer.IsRendering())
                        {
                            sceneRenderer.Stop();
                            sceneRenderer.Clear();
                        }

                        std::string configStr = exporter.Export();
                        cout << configStr;
                        config.Clear();
                        if(!config.LoadFromMemory(configStr))
                        {
                            Global::ShowErrorMessage("failed to load configure content");
                            throw std::runtime_error("");
                        }

                        if(!sceneRenderer.Start(config, scriptDir))
                        {
                            Global::ShowErrorMessage("sceneRenderer.Start returns false");
                            throw std::runtime_error("");
                        }
                    }
                    catch(...)
                    {
                        sceneRenderer.Clear();
                        Global::ShowErrorMessage("failed to start rendering");
                    }
                }
                if(ImGui::MenuItem("exit"))
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                ImGui::EndMenu();
            }

            if(ImGui::MenuItem("global"))
                openGlobalSettingWindow = true;

            if(ImGui::MenuItem("help"))
                openGlobalHelpWindow = true;

            ImGui::EndMainMenuBar();
        }

        ShowGlobalHelpWindow(openGlobalHelpWindow, keyboard);

        if(openGlobalSettingWindow)
        {
            ImGui::OpenPopup("global setting");
            ImGui::SetNextWindowSize(ImVec2(600, 800), ImGuiCond_FirstUseEver);
        }
        if(ImGui::BeginPopupModal("global setting", nullptr))
        {
            AGZ::ScopeGuard popupExitGuard([] { ImGui::EndPopup(); });

            ImGui::Text("script directory"); ImGui::SameLine();
            editorData.scriptSlot.Display(scriptBrowser);

            ImGui::Text("workspace"); ImGui::SameLine();
            editorData.workspaceSlot.Display(workspaceBrowser);

            ImGui::InputText("output filename", editorData.outputFilenameBuf.data(), editorData.outputFilenameBuf.size());

            ImGui::InputInt2("film size", &editorData.filmSize[0]);

            if(ImGui::TreeNodeEx("film filter", ImGuiTreeNodeFlags_DefaultOpen))
            {
                editorData.filmFilterSlot.Display(editorData.rscMgr);
                ImGui::TreePop();
            }

            if(ImGui::TreeNodeEx("sampler", ImGuiTreeNodeFlags_DefaultOpen))
            {
                editorData.samplerSlot.Display(editorData.rscMgr);
                ImGui::TreePop();
            }

            if(ImGui::TreeNodeEx("renderer", ImGuiTreeNodeFlags_DefaultOpen))
            {
                editorData.rendererSlot.Display(editorData.rscMgr);
                ImGui::TreePop();
            }

            if(keyboard.IsKeyPressed(AGZ::Input::KEY_ESCAPE))
                ImGui::CloseCurrentPopup();
        }

        // 计算场景管理器的位置和大小

        float sceneManagerPosX, sceneManagerPosY;
        float fbW = static_cast<float>(Global::GetFramebufferWidth());
        float fbH = static_cast<float>(Global::GetFramebufferHeight());

        {
            sceneManagerPosX = 40;
            sceneManagerPosY = ImGui::GetFrameHeight() + sceneManagerPosX * (fbH / fbW);
            ImGui::SetNextWindowPos(ImVec2(sceneManagerPosX, sceneManagerPosY), ImGuiCond_FirstUseEver);
        }

        // 场景管理器

        bool isEntityPoolDisplayed = false;
        bool isLightPoolDisplayed = false;

        if(ImGui::Begin("scene manager", nullptr, ImVec2(0, 0), -1,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
        {
            if(ImGui::BeginTabBar("scene manager tab"))
            {
                if(ImGui::BeginTabItem("entity"))
                {
                    auto &pool = editorData.rscMgr.GetPool<EntityInstance>();
                    pool.Display(editorData.rscMgr);
                    isEntityPoolDisplayed = true;
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("light"))
                {
                    auto &pool = editorData.rscMgr.GetPool<LightInstance>();
                    pool.Display(editorData.rscMgr);
                    isLightPoolDisplayed = true;
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("camera"))
                {
                    auto &pool = editorData.rscMgr.GetPool<CameraInstance>();
                    pool.Display(editorData.rscMgr);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            ImGui::End();
        }

        // 材质、纹理等对象管理器

        ImGui::SetNextWindowPos(ImVec2(sceneManagerPosX, sceneManagerPosY + 320), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("resource", nullptr, ImVec2(0, 0), -1, 
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
        {
            if(ImGui::BeginTabBar("object tab"))
            {
                if(ImGui::BeginTabItem("material"))
                {
                    auto &pool = editorData.rscMgr.GetPool<MaterialInstance>();
                    pool.Display(editorData.rscMgr);
                    if(auto mat = pool.GetSelectedInstance())
                        mat->Display(editorData.rscMgr);
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("texture"))
                {
                    auto &pool = editorData.rscMgr.GetPool<TextureInstance>();
                    pool.Display(editorData.rscMgr);
                    if(auto tex = pool.GetSelectedInstance())
                        tex->Display(editorData.rscMgr);
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("geometry"))
                {
                    auto &pool = editorData.rscMgr.GetPool<GeometryInstance>();
                    pool.Display(editorData.rscMgr);
                    if(auto data = pool.GetSelectedInstance())
                        data->Display(editorData.rscMgr);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::End();
        }

        // 控制台

        console.Display();

        // 物体属性编辑

        CameraInstance::ProjData selectedCameraProjData;

        std::shared_ptr<CameraInstance> selectedCamera = editorData.rscMgr.GetPool<CameraInstance>().GetSelectedInstance();
        ImGui::SetNextWindowPos(ImVec2(sceneManagerPosX, sceneManagerPosY + 320 + 320), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            editorData.defaultRenderingCamera.Display();
            if(selectedCamera)
                selectedCamera->Display(editorData.rscMgr);
            ImGui::End();
        }

        if(selectedCamera)
        {
            float filmAspectRatio = static_cast<float>(editorData.filmSize.x) / editorData.filmSize.y;
            selectedCameraProjData = selectedCamera->GetProjData(filmAspectRatio);
        }

        if(auto selectedEntity = editorData.rscMgr.GetPool<EntityInstance>().GetSelectedInstance(); selectedEntity && isEntityPoolDisplayed)
        {
            ImGui::SetNextWindowPos(ImVec2(sceneManagerPosX + 420, sceneManagerPosY), ImGuiCond_FirstUseEver);
            if(ImGui::Begin("property", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                selectedEntity->DisplayEx(
                    editorData.rscMgr,
                    (selectedCamera ?
                        selectedCameraProjData.projMatrix : editorData.defaultRenderingCamera.GetProjMatrix()),
                    editorData.defaultRenderingCamera.GetViewMatrix(),
                    !sceneRenderer.IsRendering());
                ImGui::End();
            }
        }

        if(auto selectedLight = editorData.rscMgr.GetPool<LightInstance>().GetSelectedInstance(); selectedLight && isLightPoolDisplayed)
        {
            ImGui::SetNextWindowPos(ImVec2(sceneManagerPosX + 420, sceneManagerPosY), ImGuiCond_FirstUseEver);
            if(ImGui::Begin("property", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                selectedLight->Display(editorData.rscMgr);
                ImGui::End();
            }
        }

        // 更新摄像机状态

        if(!ImGui::IsAnyWindowFocused())
            editorData.defaultRenderingCamera.UpdatePositionAndDirection(keyboard, mouse);
        editorData.defaultRenderingCamera.UpdateMatrix();

        auto defaultCameraProjViewMat = editorData.defaultRenderingCamera.GetProjMatrix() * editorData.defaultRenderingCamera.GetViewMatrix();

        GL::RenderContext::SetClearColor(Vec4f(Vec3f(0.2f), 0.0f));
        GL::RenderContext::ClearColorAndDepth();

        GL::RenderContext::EnableDepthTest();

        // 场景绘制

        BeginEntityRendering();
        if(selectedCamera)
        {
            auto VP = selectedCameraProjData.projMatrix * editorData.defaultRenderingCamera.GetViewMatrix();
            for(auto &ent : editorData.rscMgr.GetPool<EntityInstance>())
                ent->Render(VP, editorData.defaultRenderingCamera.GetPosition());

            SetFullViewport();
        }
        else
        {
            for(auto &ent : editorData.rscMgr.GetPool<EntityInstance>())
                ent->Render(defaultCameraProjViewMat, editorData.defaultRenderingCamera.GetPosition());
        }
        EndEntityRendering();

        GL::RenderContext::DisableDepthTest();
        GL::RenderContext::ClearDepth();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glLineWidth(3);

        if(selectedCamera)
        {
            float LTx = (std::max)(0.0f, 0.5f * fbW - 0.5f * selectedCameraProjData.viewportWidth);
            float LTy = (std::min)(fbH, 0.5f * fbH - 0.5f * selectedCameraProjData.viewportHeight);
            float RBx = (std::min)(fbW, 0.5f * fbW + 0.5f * selectedCameraProjData.viewportWidth);
            float RBy = (std::max)(0.0f, 0.5f * fbH + 0.5f * selectedCameraProjData.viewportHeight);
            imm.DrawQuadP(
                { LTx, LTy }, { RBx, RBy },
                { 1.0f, 1.0f, 1.0f, 0.3f }, false);
        }
        ScreenAxis().Display(defaultCameraProjViewMat, imm);

        glLineWidth(1);

        // 渲染结果预览

        if(sceneRenderer.IsRendering())
        {
            if(keyboard.IsKeyPressed('C'))
            {
                sceneRenderer.Stop();
                sceneRenderer.Clear();
                Global::ShowNormalMessage("rendering interrupted");
            }
        }

        if(sceneRenderer.IsRendering())
        {
            sceneRenderer.ProcessImage([&](const AGZ::Texture2D<Vec3f> &img)
            {
                sceneRendererTex.ReinitializeData(img.GetWidth(), img.GetHeight(), img.RawData());
            });
            float LTx = (std::max)(0.0f, 0.5f * fbW - 0.5f * selectedCameraProjData.viewportWidth);
            float LTy = (std::min)(fbH, 0.5f * fbH - 0.5f * selectedCameraProjData.viewportHeight);
            glViewport(
                static_cast<int>(LTx),
                static_cast<int>(LTy),
                static_cast<GLsizei>(selectedCameraProjData.viewportWidth),
                static_cast<GLsizei>(selectedCameraProjData.viewportHeight));
            imm.DrawTexturedQuad({ -1, -1 }, { 1, 1 }, sceneRendererTex);
            SetFullViewport();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glDisable(GL_BLEND);

        glfwSwapBuffers(window);
    }

    if(sceneRenderer.IsRendering())
    {
        sceneRenderer.Stop();
        sceneRenderer.Clear();
    }

    Global::_setConsole(nullptr);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    return 0;
}

int main()
{
    if(!glfwInit())
    {
        cout << "Failed to initialize glfw" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
    GLFWwindow *window = glfwCreateWindow(INIT_WIN_WIDTH, INIT_WIN_HEIGHT, "Editor", nullptr, nullptr);
    if(!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowAspectRatio(window, INIT_WIN_WIDTH, INIT_WIN_HEIGHT);

    if(glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize glew" << endl;
        return -1;
    }

    glfwSwapInterval(1);

    {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        Global::_setFramebufferWidth(w);
        Global::_setFramebufferHeight(h);
    }

    try
    {
        return Run(window);
    }
    catch(const AGZ::Exception &err)
    {
        cout << err.what() << endl;
    }
    catch(const std::exception &err)
    {
        cout << err.what() << endl;
    }
    catch(...)
    {
        cout << "???" << endl;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
