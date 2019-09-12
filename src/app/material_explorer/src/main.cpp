#include <iostream>

#include "./material_holder.h"
#include "./opengl.h"
#include "app.h"

void init_imgui(GLFWwindow *window)
{
    ImGui::CreateContext();
    ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
}

void destroy_imgui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void imgui_newframe()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void imgui_endframe()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

GLFWwindow *init_opengl()
{
    if(!glfwInit())
        throw std::runtime_error("failed to initialize glfw");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(1440, 720, "Material Explorer", nullptr, nullptr);
    if(!window)
    {
        glfwTerminate();
        throw std::runtime_error("failed to create glfw window");
    }
    glfwMakeContextCurrent(window);

    if(glewInit() != GLEW_OK)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw std::runtime_error("failed to initialize glew");
    }

    return window;
}

void destroy_opengl(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void run()
{
    auto window = init_opengl();
    AGZ_SCOPE_GUARD({ destroy_opengl(window); });

    init_imgui(window);
    AGZ_SCOPE_GUARD({ destroy_imgui(); });

    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);

    glfwSwapInterval(0);

    App app(fb_width, fb_height);

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        imgui_newframe();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(ImGui::Begin("scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            AGZ_SCOPE_GUARD({ ImGui::End(); });
            app.update(window);
        }

        app.render();

        imgui_endframe();
        glfwSwapBuffers(window);
    }
}

int main()
{
    try
    {
        run();
    }
    catch(const std::exception &err)
    {
        std::cout << err.what() << std::endl;
        return -1;
    }
}
