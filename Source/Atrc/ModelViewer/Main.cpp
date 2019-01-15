#include <iostream>

#define AGZ_ALL_IMPL
#define AGZ_USE_GLFW
#define AGZ_USE_OPENGL

#include <AGZUtils/Utils/Texture.h>

#include <GL/glew.h>
#include <GL/glfw3.h>

#include <AGZUtils/Input/GLFWCapturer.h>
#include <AGZUtils/Utils/GL.h>

using namespace std;

const char VS_SRC[] =
R"___(
#version 450 core
layout(std140) uniform WVPBlock
{
    mat4 WVP;
};
in vec2 iPos;
in vec2 iTexCoord;
out vec2 mTexCoord;
void main(void)
{
    mTexCoord = iTexCoord;
    gl_Position = WVP * vec4(iPos, 0.0, 1.0);
}
)___";

const char FS_SRC[] =
R"___(
#version 450 core
uniform sampler2D tex;
in vec2 mTexCoord;
out vec4 fragColor;
void main(void)
{
    fragColor = texture(tex, mTexCoord);
}
)___";

int Run(GLFWwindow *window)
{
    glfwSwapInterval(1);
    
    using namespace AGZ::GL;
    using namespace AGZ::Input;

    AGZ::ObjArena<> arena;

    // 注册键盘事件

    KeyboardManager<GLFWKeyboardCapturer> keyboardMgr;
    keyboardMgr.GetCapturer().Initialize(window);
    auto &keyboard = keyboardMgr.GetKeyboard();
    keyboard.AttachHandler(arena.Create<KeyDownHandler>(
        [&](const KeyDown &param)
    {
        if(param.key == KEY_ESCAPE)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    }));

    // 注册鼠标事件

    MouseManager<GLFWMouseCapturer> mouseMgr;
    mouseMgr.GetCapturer().Initialize(window);
    auto &mouse = mouseMgr.GetMouse();
    mouse.AttachHandler(arena.Create<MouseButtonDownHandler>(
        [&](const MouseButtonDown &param)
    {
        cout << "Mouse button down: " << param.button << endl;
    }));
    mouse.AttachHandler(arena.Create<CursorMoveHandler>(
        [&](const CursorMove &param)
    {
        cout << "Moving cursor: (" << param.absX << ", " << param.absY << ")" << endl;
    }));
    mouse.AttachHandler(arena.Create<WheelScrollHandler>(
        [&](const WheelScroll &param)
    {
        cout << "Scolling: " << param.offset << endl;
    }));

    // 注册窗口事件

    WindowManager<GLFWWindowCapturer> winMgr;
    winMgr.GetCapturer().Initialize(window);
    auto &win = winMgr.GetWindow();
    win.AttachHandler(arena.Create<FramebufferSizeHandler>(
        [&](const FramebufferSize &param)
    {
        glViewport(0, 0, param.w, param.h);
    }));

    // Immediate Painter

    Immediate imm;
    imm.Initialize({ 600.0f, 600.0f });

    // 编译和链接Shader

    Program program = ProgramBuilder::BuildOnce(
        VertexShader::FromMemory(VS_SRC),
        FragmentShader::FromMemory(FS_SRC));

    // 准备Vertex Buffer

    struct Vertex
    {
        Vec2f pos;
        Vec2f texCoord;
    };
    Vertex vtxData[] =
    {
        { { -0.5f, -0.5f }, { 0.0f, 0.0f } },
        { { -0.5f,  0.5f }, { 0.0f, 1.0f } },
        { {  0.5f,  0.5f }, { 1.0f, 1.0f } },
        { { -0.5f, -0.5f }, { 0.0f, 0.0f } },
        { {  0.5f,  0.5f }, { 1.0f, 1.0f } },
        { {  0.5f, -0.5f }, { 1.0f, 0.0f } }
    };
    VertexBuffer<Vertex> vtxBuf(vtxData, uint32_t(AGZ::ArraySize(vtxData)), GL_STATIC_DRAW);

    // 准备Vertex Array Object

    VertexArray vao(true);
    {
        auto pos      = program.GetAttribVariable<Vec2f>("iPos");
        auto texCoord = program.GetAttribVariable<Vec2f>("iTexCoord");
        vao.EnableAttrib(pos);
        vao.EnableAttrib(texCoord);
        vao.BindVertexBufferToAttrib(pos, vtxBuf, &Vertex::pos, 0);
        vao.BindVertexBufferToAttrib(texCoord, vtxBuf, &Vertex::texCoord, 1);
    }

    // 设置Uniform变量值

    struct WVPBlockType
    {
        Mat4f WVP;
    };
    WVPBlockType blockValue =
    {
        Mat4f::Perspective(Deg(70), 1.0f, 0.1f, 100.0f) *
        Mat4f::LookAt(
            { 0.0f, 0.0f, -1.0f },
            { 0.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f })
    };
    Std140UniformBlockBuffer<WVPBlockType> blockBuffer(&blockValue, GL_STATIC_DRAW);

    UniformVariableAssignment uniforms;
    {
        auto tex = program.GetUniformVariable<Texture2DUnit>("tex");
        auto WVPBlock = program.GetStd140UniformBlock<WVPBlockType>("WVPBlock");
        uniforms.SetValue(tex, Texture2DUnit{ 0 });
        uniforms.SetValue(WVPBlock, &blockBuffer, 0);
    }

    // 准备纹理

    Texture2D texObj(true);
    auto texData = AGZ::TextureFile::LoadRGBFromFile("./Asset/tex.png", true);
    texObj.InitializeFormatAndData(1, GL_RGB8, texData);
    texObj.Bind(0);

    // 准备采样器

    Sampler samObj(true);
    samObj.SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    samObj.SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    samObj.SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    samObj.SetParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    samObj.Bind(0);

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        keyboardMgr.Capture();
        mouseMgr.Capture();
        winMgr.Capture();

        RenderContext::ClearColorAndDepth();

        program.Bind();
        vao.Bind();
        uniforms.Bind();

        RenderContext::DrawVertices(GL_TRIANGLES, 0, vtxBuf.GetVertexCount());

        vao.Unbind();
        program.Unbind();

        imm.DrawQuad({ -0.8f, -0.8f }, { 0.8f, 0.8f }, { 0.4f, 0.4f, 0.4f, 1.0f }, false);

        glfwSwapBuffers(window);
    }

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
    GLFWwindow *window = glfwCreateWindow(640, 640, "Model Viewer", nullptr, nullptr);
    if(!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowAspectRatio(window, 1, 1);

    if(glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize glew" << endl;
        return -1;
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
