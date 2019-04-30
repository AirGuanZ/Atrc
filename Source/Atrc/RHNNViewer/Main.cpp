#include <iostream>

#include <AGZUtils/Utils/Exception.h>
#include <AGZUtils/Utils/Input.h>
#include <Atrc/RHNNViewer/App.h>
#include <Atrc/RHNNViewer/GL.h>
#include <Atrc/RHNNViewer/ReconstructGLSL.h>

using std::cout, std::endl;

void InitializeImGui(GLFWwindow *window)
{
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window);
    ImGui_ImplOpenGL3_Init();

    ImGui::StyleColorsLight();

    ImGui::GetStyle().WindowRounding = 0;
    ImGui::GetStyle().FrameBorderSize = 1;

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
    GLFWwindow *window = glfwCreateWindow(640, 480, "RHNN Viewer", nullptr, nullptr);
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

    InitializeImGui(window);

    try
    {
        App app(window);
        app.Run();
    }
    catch(const std::exception &err)
    {
        std::vector<std::string> msgs;
        AGZ::ExtractHierarchyExceptions(err, std::back_inserter(msgs));
        for(auto &m : msgs)
            cout << m << endl;
    }
    catch(...)
    {
        cout << "An unknown error occurred" << endl;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    glfwDestroyWindow(window);
    glfwTerminate();
}
