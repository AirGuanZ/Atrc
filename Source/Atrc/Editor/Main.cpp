#include <iostream>

#include <AGZUtils/Utils/Mesh.h>
#include <AGZUtils/Utils/Texture.h>

#include <Atrc/Editor/Console.h>
#include <Atrc/Editor/EditorCore.h>
#include <Atrc/Editor/GL.h>
#include <Atrc/Editor/Global.h>

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

    // Immediate painter

    GL::Immediate2D imm;
    imm.Initialize({
        static_cast<float>(Global::GetFramebufferWidth()),
        static_cast<float>(Global::GetFramebufferHeight())
    });

    // Editor core

    EditorCore editorCore;
    editorCore.Initialize();

    // Event handlers

    win.AttachHandler(arena.Create<FramebufferSizeHandler>(
        [&](const FramebufferSize &param)
    {
        Global::_setFramebufferWidth(param.w);
        Global::_setFramebufferHeight(param.h);
        editorCore.OnFramebufferResized();
        SetFullViewport();
        imm.Resize({ static_cast<float>(param.w), static_cast<float>(param.h) });
    }));
    keyboard.AttachHandler(arena.Create<KeyDownHandler>(
        [&](const KeyDown &param)
    {
        ImGui_ImplGlfw_KeyDown(param.key);
        editorCore.OnKeyDown(param.key);
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

    // Console

	Console console;
    Global::_setConsole(&console);

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

        editorCore.BeginFrame();

        editorCore.ShowMenuMenuBar();
        editorCore.ShowGlobalHelpWindow(keyboard);
        editorCore.ShowGlobalSettingWindow(keyboard);
        editorCore.ShowLoadingWindow();
        editorCore.ShowSavingWindow();

        editorCore.ShowResourceManager();
        editorCore.ShowCamera();
        editorCore.ShowEntityEditor();
        editorCore.ShowLightEditor();

        editorCore.UpdateCamera(keyboard, mouse);

        editorCore.RenderScene();
        editorCore.SaveRenderingResult();

        console.Display();
        editorCore.ShowGUI(imm, keyboard);

        editorCore.EndFrame();

        glfwSwapBuffers(window);
    }

    editorCore.Destroy();

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
