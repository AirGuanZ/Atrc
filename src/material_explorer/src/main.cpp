#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define AGZ_ENABLE_OPENGL
#include <agz/utility/opengl.h>

int main()
{
    if(!glfwInit())
        return -1;

    GLFWwindow *window = glfwCreateWindow(1200, 900, "Hello World", nullptr, nullptr);
    if(!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if(glewInit() != GLEW_OK)
    {
        std::cout << "failed to initialize glew" << std::endl;
        return -1;
    }

    while(!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
