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

void InitializeImGui(GLFWwindow *window)
{
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window);
    ImGui_ImplOpenGL3_Init();

    ImGui::StyleColorsDark();

    ImGui::GetStyle().WindowRounding = 0;

    ImFontConfig fontConfig;
    fontConfig.SizePixels = 16;
    ImGui::GetIO().Fonts->AddFontDefault(&fontConfig);
    
    /*ImFontConfig config;
    config.MergeMode = true;
    ImGui::GetIO().Fonts->AddFontFromFileTTF(
        "./InputSerifCondensed-Italic.ttf", 16);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(
        "./songti.ttf", 16, &config, ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
    ImGui::GetIO().Fonts->Build();*/

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::EndFrame();
}

int Run(GLFWwindow *window)
{
    using namespace AGZ::GraphicsAPI;
    using namespace AGZ::Input;

    AGZ::ObjArena<> arena;
    
    // 初始化IMGUI

    InitializeImGui(window);

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
        static_cast<float>(Global::FbW()),
        static_cast<float>(Global::FbH())
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
        glViewport(0, 0, Global::FbW(), Global::FbH());
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
        editorCore.ShowExportingSH2DSceneWindow();
        editorCore.ShowExportingSH2DLightWindow();
        editorCore.ShowLoadingWindow();
        editorCore.ShowSavingWindow();

        ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_FirstUseEver);
        if(editorCore.BeginLeftWindow())
        {
            editorCore.ShowResourceManager();
            editorCore.EndLeftWindow();
        }

        ImGui::SetNextWindowSize(ImVec2(450, 100), ImGuiCond_FirstUseEver);
        if(editorCore.BeginRightWindow())
        {
            if(ImGui::BeginTabBar("property", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_Reorderable))
            {
                if(ImGui::BeginTabItem("entity"))
                {
                    ImGui::BeginChild("");
                    editorCore.ShowEntityEditor();
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("light"))
                {
                    ImGui::BeginChild("");
                    editorCore.ShowLightEditor();
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("camera"))
                {
                    ImGui::BeginChild("");
                    editorCore.ShowCamera();
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            editorCore.EndRightWindow();
        }

        ImGui::SetNextWindowSize(ImVec2(200, 350), ImGuiCond_FirstUseEver);
        if(editorCore.BeginBottomWindow())
        {
            if(ImGui::BeginTabBar("bottom window tab"))
            {
                AGZ::ScopeGuard endTabBar([] { ImGui::EndTabBar(); });
                if(ImGui::BeginTabItem("console"))
                {
                    AGZ::ScopeGuard endTabItem([] { ImGui::EndTabItem(); });
                    ImGui::BeginChild("");
                    console.Display();
                    ImGui::EndChild();
                }
                if(ImGui::BeginTabItem("global"))
                {
                    AGZ::ScopeGuard endTabItem([] { ImGui::EndTabItem(); });
                    ImGui::BeginChild("");
                    editorCore.ShowGlobalSettings();
                    ImGui::EndChild();
                }
                if(ImGui::BeginTabItem("rendering"))
                {
                    AGZ::ScopeGuard endTabItem([] { ImGui::EndTabItem(); });
                    ImGui::BeginChild("");
                    editorCore.ShowRenderingSettings();
                    ImGui::EndChild();
                }
            }
            editorCore.EndBottomWindow();
        }

        Global::_setPreviewWindow(
            static_cast<int>(editorCore.GetViewportX()),
            static_cast<int>(editorCore.GetViewportY()),
            static_cast<int>(editorCore.GetViewportWidth()),
            static_cast<int>(editorCore.GetViewportHeight()));

        editorCore.UpdateCamera(keyboard, mouse);

        editorCore.RenderScene();
        editorCore.SaveRenderingResult();
        editorCore.ShowPreviewGUI(imm, keyboard);

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
