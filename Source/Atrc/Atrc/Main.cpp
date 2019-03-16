#include <iostream>

#include <Atrc/Atrc/ResourceInterface/ResourceSlot.h>
#include <Atrc/Atrc/FilmFilter/FilmFilter.h>
#include <Atrc/Atrc/Texture1/Texture1.h>
#include <Atrc/Atrc/Window.h>

#include <Lib/imgui/imgui/imgui.h>
#include <Lib/ImFileBrowser/imfilebrowser.h>

void InitializeImGui(GLFWwindow *window)
{
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window);
    ImGui_ImplOpenGL3_Init();

    ImGui::StyleColorsDark();

    ImGui::GetStyle().WindowRounding = 0;

    ImGui::GetIO().Fonts->AddFontFromFileTTF(
        "./font.ttf", 16, nullptr, ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
}

int Run(GLFWwindow *window)
{
    Window winInfo;
    {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        winInfo.SetFBSize(w, h);
    }

    InitializeImGui(window);
    AGZ::ScopeGuard imguiShutdownGuard([]
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
    });

    AGZ::Input::KeyboardManager<AGZ::Input::GLFWKeyboardCapturer> keyboardMgr;
    keyboardMgr.GetCapturer().Initialize(window);
    auto &keyboard = keyboardMgr.GetKeyboard();

    AGZ::Input::MouseManager<AGZ::Input::GLFWMouseCapturer> mouseMgr;
    mouseMgr.GetCapturer().Initialize(window);
    auto &mouse = mouseMgr.GetMouse();

    AGZ::Input::WindowManager<AGZ::Input::GLFWWindowCapturer> winMgr;
    winMgr.GetCapturer().Initialize(window);
    auto &win = winMgr.GetWindow();

    AGZ::ObjArena<> handlerArena;

    win.AttachHandler(handlerArena.Create<AGZ::Input::FramebufferSizeHandler>([&](const AGZ::Input::FramebufferSize &param)
    {
        winInfo.SetFBSize(param.w, param.h);
        glViewport(0, 0, param.w, param.h);
    }));

    keyboard.AttachHandler(handlerArena.Create<AGZ::Input::KeyDownHandler>(
        [&](const AGZ::Input::KeyDown &param)
    {
        ImGui_ImplGlfw_KeyDown(param.key);
    }));

    keyboard.AttachHandler(handlerArena.Create<AGZ::Input::KeyUpHandler>(
        [&](const AGZ::Input::KeyUp &param)
    {
        ImGui_ImplGlfw_KeyUp(param.key);
    }));

    keyboard.AttachHandler(handlerArena.Create<AGZ::Input::CharEnterHandler>(
        [&](const AGZ::Input::CharEnter &param)
    {
        ImGui_ImplGlfw_Char(param.ch);
    }));

    mouse.AttachHandler(handlerArena.Create<AGZ::Input::MouseButtonDownHandler>(
        [&](const AGZ::Input::MouseButtonDown &param)
    {
        ImGui_ImplGlfw_MouseButtonDown(param.button);
    }));

    mouse.AttachHandler(handlerArena.Create<AGZ::Input::WheelScrollHandler>(
        [&](const AGZ::Input::WheelScroll &param)
    {
        ImGui_ImplGlfw_WheelScroll(param.offset);
    }));

    ResourceCreatorManagerList ctrMgrList;
    RegisterFilmFilterCreators(ctrMgrList.GetCreatorMgr<FilmFilterInstance>());
    RegisterTexture1Creators(ctrMgrList.GetCreatorMgr<Texture1Instance>());

    ResourceCreateContext ctrCtx = { &ctrMgrList };

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ResourceSlot<FilmFilterInstance> filmFilterSlot(ctrCtx, "Box");
    ResourceSlot<Texture1Instance> tex1Slot(ctrCtx);
    tex1Slot.SetRscTypeChangedCallback([](Texture1Instance &i) {i.SetRange(0, 1); }, true);

    ImGui::EndFrame();

    float bottomWindowSizeY = 100;
    float rightWindowSizeX = 100;

    ImGui::FileBrowser fileBrowser(ImGuiFileBrowserFlags_CloseOnEsc | ImGuiFileBrowserFlags_NoModal);
    
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        keyboardMgr.Capture();
        mouseMgr.Capture();
        winMgr.Capture();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // four windows

        // top

        float topWindowSizeY = 0;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSizeConstraints(
            ImVec2(winInfo.FbWf(), 50),
            ImVec2(winInfo.FbWf(), winInfo.FbHf() / 2 - 100));
        if(ImGui::Begin("Top Window", nullptr,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResizeGrip | ImGuiWindowFlags_NoTitleBar))
        {
            topWindowSizeY = ImGui::GetWindowSize().y;
            ImGui::End();
        }

        // bottom

        ImGui::SetNextWindowSizeConstraints(
            ImVec2(winInfo.FbWf(), 50),
            ImVec2(winInfo.FbWf(), winInfo.FbHf() / 2 - 100));
        ImGui::SetNextWindowPos(ImVec2(0, winInfo.FbHf() - bottomWindowSizeY));
        if(ImGui::Begin("Bottom Window", nullptr,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResizeGrip | ImGuiWindowFlags_NoTitleBar))
        {
            bottomWindowSizeY = ImGui::GetWindowSize().y;
            ImGui::End();
        }

        // left

        float middleWindowSizeY = winInfo.FbHf() - topWindowSizeY - bottomWindowSizeY;
        float leftWindowSizeX = 0;

        ImGui::SetNextWindowPos(ImVec2(0, topWindowSizeY));
        ImGui::SetNextWindowSizeConstraints(
            ImVec2(100, middleWindowSizeY),
            ImVec2(winInfo.FbWf() / 2 - 100, middleWindowSizeY));
        if(ImGui::Begin("Left Window", nullptr,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResizeGrip | ImGuiWindowFlags_NoTitleBar))
        {
            filmFilterSlot.Display(ctrCtx);
            if(ImGui::Button(u8"文件"))
                fileBrowser.Open();
            leftWindowSizeX = ImGui::GetWindowSize().x;
            ImGui::End();
        }

        fileBrowser.Display();

        if(fileBrowser.HasSelected())
        {
            std::cout << fileBrowser.GetSelected().string() << std::endl;
            fileBrowser.ClearSelected();
        }

        // right

        ImGui::SetNextWindowSizeConstraints(
            ImVec2(100, middleWindowSizeY),
            ImVec2(winInfo.FbWf() / 2 - 100, middleWindowSizeY));
        ImGui::SetNextWindowPos(ImVec2(winInfo.FbWf() - rightWindowSizeX, topWindowSizeY));
        if(ImGui::Begin("Right Window", nullptr,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResizeGrip | ImGuiWindowFlags_NoTitleBar))
        {
            tex1Slot.Display(ctrCtx);
            rightWindowSizeX = ImGui::GetWindowSize().x;
            ImGui::End();
        }

        winInfo.SetPvSize(
            static_cast<int>(winInfo.FbWf() - leftWindowSizeX - rightWindowSizeX),
            static_cast<int>(winInfo.FbHf() - topWindowSizeY - bottomWindowSizeY));

        GL::RenderContext::ClearColorAndDepth();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    return 0;
}

int main()
{
    if(!glfwInit())
    {
        std::cout << "Failed to initialize glfw" << std::endl;
        return -1;
    }
    AGZ::ScopeGuard glfwTerminateGuard([] { glfwTerminate(); });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow *window = glfwCreateWindow(640, 480, "Atrc", nullptr, nullptr);
    if(!window)
        return -1;
    AGZ::ScopeGuard glfwDestroyWindowGuard([=] { glfwDestroyWindow(window); });

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if(glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize glew" << std::endl;
        return -1;
    }

    try
    {
        return Run(window);
    }
    catch(const std::exception &err)
    {
        std::cout << err.what() << std::endl;
    }
    catch(...)
    {
        std::cout << "An unknown error occurred...=" << std::endl;
    }
}
