#include <iostream>

#define AGZ_ALL_IMPL

#include <AGZUtils/Utils/Texture.h>

#include <GL/glew.h>
#include <GL/glfw3.h>

#include <AGZUtils/Input/GLFWCapturer.h>
#include <AGZUtils/Utils/GL.h>

using namespace std;

const char VS_SRC[] =
R"___(
#version 450 core
in vec2 pos;
in vec2 iTexCoord;
out vec2 mTexCoord;
void main(void)
{
    mTexCoord = iTexCoord;
    gl_Position = vec4(pos, 0.0, 1.0);
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

int Run()
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

    if(glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize glew" << endl;
        return -1;
    }

    glfwSwapInterval(1);
    
    {
        using namespace AGZ::GL;
        using namespace AGZ::Input;

        AGZ::ObjArena<> arena;

        KeyboardManager<GLFWKeyboardCapturer> input;
        input.GetCapturer().Initialize(window);
        input.GetKeyboard().AttachHandler(arena.Create<KeyDownHandler>(
            [&](const KeyDown &param)
        {
            if(param.key == KEY_ESCAPE)
                glfwSetWindowShouldClose(window, GLFW_TRUE);
        }));

        VertexShader vs;
        FragmentShader fs;

        if(!vs.LoadFromMemory(VS_SRC))
        {
            cout << vs.GetErrMsg().ToStdString() << endl;
            return -1;
        }

        if(!fs.LoadFromMemory(FS_SRC))
        {
            cout << fs.GetErrMsg().ToStdString() << endl;
            return -1;
        }

        ProgramBuilder programBuilder(vs, fs);
        Program program = programBuilder.Build();

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
        VertexBuffer<Vertex> vtxBuf(vtxData, AGZ::ArraySize(vtxData), GL_STATIC_DRAW);

        VertexArrayObject vao(true);

        AttribVariable<Vec2f> pos = program.GetAttribVariable<Vec2f>("pos");
        AttribVariable<Vec2f> texCoord = program.GetAttribVariable<Vec2f>("iTexCoord");
        vao.EnableAttrib(pos);
        vao.EnableAttrib(texCoord);
        vao.BindVertexBufferToAttrib(pos, vtxBuf, &Vertex::pos, 0);
        vao.BindVertexBufferToAttrib(texCoord, vtxBuf, &Vertex::texCoord, 1);

        UniformVariable<Texture2DUnit> tex = program.GetUniformVariable<Texture2DUnit>("tex");
        UniformVariableAssignment uniforms;
        uniforms.SetValue(tex, Texture2DUnit{ 0 });

        Texture2D texObj;
        texObj.InitializeHandle();
        auto texData = AGZ::Texture2D<Vec3b>(AGZ::TextureFile::LoadRGBFromFile("./Asset/tex.png", true));
        texObj.InitializeFormatAndData(1, texData.GetWidth(), texData.GetHeight(), GL_RGB8, texData.RawData());
        texObj.SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        texObj.SetParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        texObj.Bind(0);

        glClearColor(0.0f, 1.0f, 1.0f, 0.0f);

        while(!glfwWindowShouldClose(window))
        {
            glClear(GL_COLOR_BUFFER_BIT);

            program.Bind();
            vao.Bind();
            uniforms.Bind();
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vtxBuf.GetVertexCount()));
            vao.Unbind();
            program.Unbind();

            glfwSwapBuffers(window);
            glfwPollEvents();

            input.Capture();
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

int main()
{
    try
    {
        return Run();
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
}
