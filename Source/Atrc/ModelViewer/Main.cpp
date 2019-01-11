#include <glfw3.h>
#include <gl.h>

#include <AGZUtils/Utils/Alloc.h>
#include <AGZUtils/Utils/Input.h>

using namespace std;

int main()
{
    if(!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow *window = glfwCreateWindow(640, 480, "Model Viewer", nullptr, nullptr);
    if(!window)
    {
        glfwTerminate();
        return -1;
    }

    using namespace AGZ::Input;

    KeyboardManager<GLFWKeyboardCapturer> input;

    input.GetCapturer().Initialize(window);
    AGZ::ObjArena<> arena;
    input.GetKeyboard().AttachHandler(arena.Create<KeyDownHandler>(
        [&](const KeyDown &param)
    {
        if(param.key == KEY_ESCAPE)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    }));

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glClearColor(0.0f, 1.0f, 1.0f, 0.0f);

    while(!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
        input.Capture();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
