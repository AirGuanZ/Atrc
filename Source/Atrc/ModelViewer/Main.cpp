#include <iostream>

#include <AGZUtils/Utils/Mesh.h>
#include <AGZUtils/Utils/Texture.h>

#include "Camera.h"
#include "Console.h"
#include "GL.h"
#include "Global.h"
#include "ModelManager.h"
#include "ScreenAxis.h"
#include "TransformController.h"

using namespace std;

constexpr int INIT_WIN_WIDTH = 1400;
constexpr int INIT_WIN_HEIGHT = 900;

void ShowGlobalHelpWindow(bool open, const AGZ::Input::Keyboard &kb)
{
    if(open)
        ImGui::OpenPopup("global help");
    ImGui::SetNextWindowSize(ImVec2(640, 640), ImGuiCond_Always);
    if(!ImGui::BeginPopupModal("global help", nullptr, ImGuiWindowFlags_NoResize))
        return;
    AGZ::ScopeGuard windowExitGuard([] { ImGui::EndPopup(); });

    static const char *MSG =
u8R"____(Hold mouse middle button to adjust camera direction when there is no focused window.
Press ESC to close this help window.)____";
    ImGui::TextWrapped("%s", MSG);

    if(kb.IsKeyPressed(AGZ::Input::KEY_ESCAPE))
        ImGui::CloseCurrentPopup();
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

    ImGui::GetStyle().PopupRounding   = 12;
    ImGui::GetStyle().WindowRounding  = 12;
    ImGui::GetStyle().GrabRounding    = 12;
    ImGui::GetStyle().FrameRounding   = 12;
    ImGui::GetStyle().FrameBorderSize = 1;

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
        auto &global = Global::GetInstance();

        screen2DFramebuffer   = GL::FrameBuffer(true);
        screen2DDepthBuffer   = GL::RenderBuffer(true);
        screen2DRenderTexture = GL::Texture2D(true);

        screen2DDepthBuffer.SetFormat(global.framebufferWidth, global.framebufferHeight, GL_DEPTH_COMPONENT);
        screen2DRenderTexture.InitializeFormat(1, global.framebufferWidth, global.framebufferHeight, GL_RGBA8);
        screen2DFramebuffer.Attach(GL_COLOR_ATTACHMENT0, screen2DRenderTexture);
        screen2DFramebuffer.Attach(GL_DEPTH_ATTACHMENT, screen2DDepthBuffer);

        AGZ_ASSERT(screen2DFramebuffer.IsComplete());
    };
    ReinitializeScreen2DFramebuffer();

    // 注册窗口事件

    win.AttachHandler(arena.Create<FramebufferSizeHandler>(
        [&](const FramebufferSize &param)
    {
        auto &global = Global::GetInstance();
        global.framebufferWidth = param.w;
        global.framebufferHeight = param.h;
        glViewport(0, 0, param.w, param.h);
        imm.Resize({ static_cast<float>(param.w), static_cast<float>(param.h) });
        ReinitializeScreen2DFramebuffer();
    }));

    // ImGui Font Size

    ImFontConfig defaultFontConfig;
    defaultFontConfig.SizePixels = 16.0f;
    ImGui::GetIO().Fonts->AddFontDefault(&defaultFontConfig);
    
    // Model Manager

	Camera camera("default");

	Console console;
	ModelManager modelMgr;

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        keyboardMgr.Capture();
        mouseMgr.Capture();
        winMgr.Capture();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        TransformController::BeginFrame();

        bool openGlobalHelpWindow = false;

        if(ImGui::BeginMainMenuBar())
        {
            if(ImGui::BeginMenu("file"))
            {
                if(ImGui::MenuItem("exit"))
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("help"))
            {
                if(ImGui::MenuItem("global"))
                    openGlobalHelpWindow = true;
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        {
            float fbW = static_cast<float>(Global::GetInstance().framebufferWidth);
            float fbH = static_cast<float>(Global::GetInstance().framebufferHeight);
            float posX = 40;
            float posY = ImGui::GetFrameHeight() + posX * (fbH / fbW);
            ImGui::SetNextWindowPos(ImVec2(posX, posY), ImGuiCond_FirstUseEver);
        }

        if(ImGui::Begin("Scene Manager", nullptr, ImVec2(400, 700)))
        {
            if(ImGui::BeginTabBar("scene manager tab"))
            {
                if(ImGui::BeginTabItem("model"))
                {
                    modelMgr.Display(console, camera);
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

        console.Display();

        ShowGlobalHelpWindow(openGlobalHelpWindow, keyboard);

        if(!ImGui::IsAnyWindowFocused())
            camera.UpdatePositionAndDirection(keyboard, mouse);

        screen2DFramebuffer.Bind();
        GL::RenderContext::ClearColorAndDepth();

        glLineWidth(3);
        ScreenAxis().Display(camera, imm);
        glLineWidth(1);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        screen2DFramebuffer.Unbind();

        GL::RenderContext::SetClearColor(Vec4f(Vec3f(0.4f), 0.0f));
        GL::RenderContext::ClearColorAndDepth();

        GL::RenderContext::EnableDepthTest();

        modelMgr.Render(camera);

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
        imm.DrawTexturedTriangles(scrQuadVtx, 6, screen2DRenderTexture);
        glDisable(GL_BLEND);

        glfwSwapBuffers(window);
    }

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
    GLFWwindow *window = glfwCreateWindow(INIT_WIN_WIDTH, INIT_WIN_HEIGHT, "Model Viewer", nullptr, nullptr);
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
        auto &global = Global::GetInstance();
        glfwGetFramebufferSize(window, &global.framebufferWidth, &global.framebufferHeight);
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
