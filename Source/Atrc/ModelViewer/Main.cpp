#include <iostream>

#include <AGZUtils/Utils/Mesh.h>
#include <AGZUtils/Utils/Texture.h>

#include <Atrc/ModelViewer/ResourceManagement/EntityCreator.h>
#include <Atrc/ModelViewer/ResourceManagement/ResourceManager.h>
#include <Atrc/ModelViewer/Camera.h>
#include <Atrc/ModelViewer/Console.h>
#include <Atrc/ModelViewer/FilenameSlot.h>
#include <Atrc/ModelViewer/GL.h>
#include <Atrc/ModelViewer/Global.h>
#include <Atrc/ModelViewer/LauncherScriptExporter.h>
#include <Atrc/ModelViewer/ScreenAxis.h>
#include <Atrc/ModelViewer/TransformController.h>

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

void SetRenderingViewport()
{
    auto dh = ImGui::GetFrameHeight();
    glViewport(0, 0, Global::GetFramebufferWidth(), Global::GetFramebufferHeight() - static_cast<GLsizei>(dh));
}

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

    Global::_setMenuBarHeight(static_cast<int>(ImGui::GetFrameHeight()));

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
    imm.Initialize({ 600.0f, 600.0f });

    // Screen2D Framebuffer

    GL::FrameBuffer screen2DFramebuffer(true);
    GL::RenderBuffer screen2DDepthBuffer(true);
    GL::Texture2D screen2DRenderTexture(true);

    auto ReinitializeScreen2DFramebuffer = [&]
    {
        screen2DFramebuffer   = GL::FrameBuffer(true);
        screen2DDepthBuffer   = GL::RenderBuffer(true);
        screen2DRenderTexture = GL::Texture2D(true);

        screen2DDepthBuffer.SetFormat(Global::GetFramebufferWidth(), Global::GetFramebufferHeight(), GL_DEPTH_COMPONENT);
        screen2DRenderTexture.InitializeFormat(1, Global::GetFramebufferWidth(), Global::GetFramebufferHeight(), GL_RGBA8);
        screen2DFramebuffer.Attach(GL_COLOR_ATTACHMENT0, screen2DRenderTexture);
        screen2DFramebuffer.Attach(GL_DEPTH_ATTACHMENT, screen2DDepthBuffer);

        //AGZ_ASSERT(screen2DFramebuffer.IsComplete());
    };
    ReinitializeScreen2DFramebuffer();

    // 注册窗口事件

    win.AttachHandler(arena.Create<FramebufferSizeHandler>(
        [&](const FramebufferSize &param)
    {
        Global::_setFramebufferWidth(param.w);
        Global::_setFramebufferHeight(param.h);
        imm.Resize({ static_cast<float>(param.w), static_cast<float>(param.h) });
        ReinitializeScreen2DFramebuffer();
    }));
    
    // Model Manager

    DefaultRenderingCamera camera("default");

	Console console;
    Global::_setConsole(&console);

    // Object Manager

    ResourceManager rscMgr;
    RegisterResourceCreators(rscMgr);

    // global setting

    Vec2i filmSize = { 640, 480 };
    FilmFilterSlot filmFilterSlot;
    SamplerSlot samplerSlot;
    RendererSlot rendererSlot;
    CameraSlot cameraSlot;

    char outputFilenameBuf[512] = "$Output.png";

    TFilenameSlot<false, FilenameMode::Absolute> scriptSlot;
    FileBrowser scriptBrowser("script", true, "");

    TFilenameSlot<false, FilenameMode::RelativeToScript> workspaceSlot;
    FileBrowser workspaceBrowser("workspace", true, "");

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
        TransformController::BeginFrame();

        // 全局菜单栏

        bool openGlobalHelpWindow = false;
        bool openGlobalSettingWindow = false;

        if(ImGui::BeginMainMenuBar())
        {
            if(ImGui::BeginMenu("file"))
            {
                if(ImGui::MenuItem("export"))
                {
                    std::string scriptDir = scriptSlot.GetExportedFilename("", "");
                    std::string workspaceDir = workspaceSlot.GetExportedFilename("", scriptDir);
                    LauncherScriptExportingContext ctx(
                        &camera,
                        cameraSlot.GetInstance().get(),
                        filmFilterSlot.GetInstance().get(),
                        rendererSlot.GetInstance().get(),
                        samplerSlot.GetInstance().get(),
                        workspaceDir,
                        scriptDir,
                        outputFilenameBuf,
                        filmSize);
                    LauncherScriptExporter exporter(rscMgr, ctx);
                    std::cout << exporter.Export();
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
            scriptSlot.Display(scriptBrowser);

            ImGui::Text("workspace"); ImGui::SameLine();
            workspaceSlot.Display(workspaceBrowser);

            ImGui::InputText("output filename", outputFilenameBuf, AGZ::ArraySize(outputFilenameBuf));

            ImGui::InputInt2("film size", &filmSize[0]);

            if(ImGui::TreeNodeEx("film filter", ImGuiTreeNodeFlags_DefaultOpen))
            {
                filmFilterSlot.Display(rscMgr);
                ImGui::TreePop();
            }

            if(ImGui::TreeNodeEx("camera", ImGuiTreeNodeFlags_DefaultOpen))
            {
                cameraSlot.Display(rscMgr);
                ImGui::TreePop();
            }

            if(ImGui::TreeNodeEx("sampler", ImGuiTreeNodeFlags_DefaultOpen))
            {
                samplerSlot.Display(rscMgr);
                ImGui::TreePop();
            }

            if(ImGui::TreeNodeEx("renderer", ImGuiTreeNodeFlags_DefaultOpen))
            {
                rendererSlot.Display(rscMgr);
                ImGui::TreePop();
            }

            if(keyboard.IsKeyPressed(AGZ::Input::KEY_ESCAPE))
                ImGui::CloseCurrentPopup();
        }

        // 计算场景管理器的位置和大小

        float sceneManagerPosX, sceneManagerPosY;

        {
            float fbW = static_cast<float>(Global::GetFramebufferWidth());
            float fbH = static_cast<float>(Global::GetFramebufferHeight());
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
                    auto &pool = rscMgr.GetPool<EntityInstance>();
                    pool.Display();
                    isEntityPoolDisplayed = true;
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("light"))
                {
                    auto &pool = rscMgr.GetPool<LightInstance>();
                    pool.Display();
                    isLightPoolDisplayed = true;
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("camera"))
                {
                    camera.Display();
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
                    auto &pool = rscMgr.GetPool<MaterialInstance>();
                    pool.Display();
                    if(auto mat = pool.GetSelectedInstance())
                        mat->Display(rscMgr);
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("texture"))
                {
                    auto &pool = rscMgr.GetPool<TextureInstance>();
                    pool.Display();
                    if(auto tex = pool.GetSelectedInstance())
                        tex->Display(rscMgr);
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("geometry"))
                {
                    auto &pool = rscMgr.GetPool<GeometryInstance>();
                    pool.Display();
                    if(auto data = pool.GetSelectedInstance())
                        data->Display(rscMgr);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::End();
        }

        // 控制台

        console.Display();

        // 物体属性编辑

        if(auto selectedEntity = rscMgr.GetPool<EntityInstance>().GetSelectedInstance(); selectedEntity && isEntityPoolDisplayed)
        {
            ImGui::SetNextWindowPos(ImVec2(sceneManagerPosX + 420, sceneManagerPosY), ImGuiCond_FirstUseEver);
            if(ImGui::Begin("model/light", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                selectedEntity->Display(rscMgr);
                selectedEntity->DisplayTransform(camera.GetProjMatrix(), camera.GetViewMatrix());
                ImGui::End();
            }
        }
        
        if(auto selectedLight = rscMgr.GetPool<LightInstance>().GetSelectedInstance(); selectedLight && isLightPoolDisplayed)
        {
            ImGui::SetNextWindowPos(ImVec2(sceneManagerPosX + 420, sceneManagerPosY), ImGuiCond_FirstUseEver);
            if(ImGui::Begin("model/light", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                selectedLight->Display(rscMgr);
                ImGui::End();
            }
        }

        // 更新摄像机状态

        if(!ImGui::IsAnyWindowFocused())
            camera.UpdatePositionAndDirection(keyboard, mouse);

        // 屏幕上二维对象的绘制

        auto cameraProjViewMat = camera.GetProjMatrix() * camera.GetViewMatrix();
        Mat4f screenProjMat = Mat4f::Perspective(Deg(60), Global::GetWindowAspectRatio(), 0.1f, 100.0f);

        {
            screen2DFramebuffer.Bind();
            GL::RenderContext::ClearColorAndDepth();

            glLineWidth(3);
            ScreenAxis().Display(screenProjMat * camera.GetViewMatrix(), imm);
            glLineWidth(1);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            screen2DFramebuffer.Unbind();
        }

        GL::RenderContext::SetClearColor(Vec4f(Vec3f(0.2f), 0.0f));
        GL::RenderContext::ClearColorAndDepth();

        GL::RenderContext::EnableDepthTest();

        SetRenderingViewport();

        // 场景绘制

        BeginEntityRendering();
        for(auto &ent : rscMgr.GetPool<EntityInstance>())
            ent->Render(cameraProjViewMat);
        EndEntityRendering();

        // 把屏幕上的二维内容叠加到screen buffer上

        static const GL::Immediate2D::TexturedVertex scrQuadVtx[] =
        {
            { { -1.0f, -1.0f }, { 0.0f, 0.0f } },
            { { -1.0f, 1.0f },  { 0.0f, 1.0f } },
            { { 1.0f, 1.0f },   { 1.0f, 1.0f } },
            { { -1.0f, -1.0f }, { 0.0f, 0.0f } },
            { { 1.0f, 1.0f },   { 1.0f, 1.0f } },
            { { 1.0f, -1.0f },  { 1.0f, 0.0f } }
        };

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        SetFullViewport();
        imm.DrawTexturedTriangles(scrQuadVtx, 6, screen2DRenderTexture);
        glDisable(GL_BLEND);

        glfwSwapBuffers(window);
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
    GLFWwindow *window = glfwCreateWindow(INIT_WIN_WIDTH, INIT_WIN_HEIGHT, "ModelViewer", nullptr, nullptr);
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
