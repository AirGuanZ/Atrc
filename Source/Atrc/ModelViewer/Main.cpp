#include <iostream>

#include <AGZUtils/Utils/Mesh.h>
#include <AGZUtils/Utils/Texture.h>

#include "Console.h"
#include "GL.h"
#include "ModelRenderer.h"

using namespace std;
using AGZ::Str8;

constexpr int INIT_WIN_WIDTH = 1200;
constexpr int INIT_WIN_HEIGHT = 700;

constexpr int MODEL_VIEW_WIDTH = 300;
constexpr int MODEL_VIEW_HEIGHT = 300;

struct ModelTransform
{
    Vec3f translate;
    Deg rotateZ = Deg(0);
    Deg rotateY = Deg(0);
    Deg rotateX = Deg(0);
    float scale = 1;
    Vec3f preTranslate;
};

// （在imgui窗口内调用）模型集合变换参数设置面板
void SetModelTransform(Console &console, ModelTransform *trans)
{
    AGZ_ASSERT(trans);

    if(ImGui::InputFloat3("Offset", &trans->preTranslate[0], 4, ImGuiInputTextFlags_EnterReturnsTrue))
        console.AddText(ConsoleText::Normal, "Offset := " + AGZ::ToStr8(trans->preTranslate));

    if(ImGui::InputFloat("Scale", &trans->scale, 0.01f, 1.0f, "%.4f", ImGuiInputTextFlags_EnterReturnsTrue))
    {
        trans->scale = std::max(trans->scale, 0.001f);
        console.AddText(ConsoleText::Normal, "Scale := " + AGZ::ToStr8(trans->scale));
    }

    if(ImGui::InputFloat("RotateX", &trans->rotateX.value, 0.5f, 5.0f, "%.4f", ImGuiInputTextFlags_EnterReturnsTrue))
        console.AddText(ConsoleText::Normal, "RotateX := " + AGZ::ToStr8(trans->rotateX.value));
    if(ImGui::InputFloat("RotateZ", &trans->rotateZ.value, 0.5f, 5.0f, "%.4f", ImGuiInputTextFlags_EnterReturnsTrue))
        console.AddText(ConsoleText::Normal, "RotateZ := " + AGZ::ToStr8(trans->rotateZ.value));
    if(ImGui::InputFloat("RotateY", &trans->rotateY.value, 0.5f, 5.0f, "%.4f", ImGuiInputTextFlags_EnterReturnsTrue))
        console.AddText(ConsoleText::Normal, "RotateY := " + AGZ::ToStr8(trans->rotateY.value));

    if(ImGui::InputFloat3("Translate", &trans->translate[0], 4, ImGuiInputTextFlags_EnterReturnsTrue))
        console.AddText(ConsoleText::Normal, "Translate := " + AGZ::ToStr8(trans->translate));
}

// 从指定路径加载模型数据
bool ReloadModelData(const Str8 &filename, ModelRenderer *model)
{
    std::vector<ModelRenderer::Vertex> modelVtxData;
    std::vector<uint32_t> modelElemData;
    {
        AGZ::Mesh::WavefrontObj<float> obj;
        if(!obj.LoadFromFile(filename))
        {
            cout << "Failed to load model data..." << endl;
            return false;
        }
        auto meshGrp = obj.ToGeometryMeshGroup();
        obj.Clear();

        for(auto &it : meshGrp.submeshes)
        {
            for(auto &v : it.second.vertices)
            {
                ModelRenderer::Vertex nv;
                nv.pos = v.pos;
                nv.nor = v.nor;
                nv.tex = v.tex.uv();
                modelVtxData.push_back(nv);
                modelElemData.push_back(uint32_t(modelElemData.size()));
            }
        }
    }

    model->SetModelData(
        modelVtxData.data(), uint32_t(modelVtxData.size()),
        modelElemData.data(), uint32_t(modelElemData.size()));

    return true;
}

int Run(GLFWwindow *window)
{
    using namespace AGZ::GraphicsAPI;
    using namespace AGZ::Input;

    AGZ::ObjArena<> arena;

    Console console;

    // 初始化IMGUI

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window);
    ImGui_ImplOpenGL3_Init();

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
        if(param.key == KEY_ESCAPE)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    }));
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

    // 注册窗口事件

    win.AttachHandler(arena.Create<FramebufferSizeHandler>(
        [&](const FramebufferSize &param)
    {
        glViewport(0, 0, param.w, param.h);
    }));

    // Immediate Painter

    GL::Immediate imm;
    imm.Initialize({ 600.0f, 600.0f });

    // Model Renderer

    ModelRenderer model;
    model.Initialize();

    if(!ReloadModelData("Scene/Asset/Test/Mitsuba.obj", &model))
    {
        console.AddText(ConsoleText::Error, "Failed to load model data...");
        cout << "Failed to load model data..." << endl;
        return -1;
    }

    GL::Texture2D modelTex(true);
    model.SetTexture(&modelTex);

    model.SetView(Mat4f::LookAt({ -4.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }));
    model.SetProj(Mat4f::Perspective(Deg(60.0f), 1.0f, 0.1f, 1000.0f));

    // 准备FBO

    GL::FrameBuffer fbo(true);
    GL::RenderBuffer rbo(true);
    GL::Texture2D fbTex(true);
    fbTex.InitializeFormat(1, MODEL_VIEW_WIDTH, MODEL_VIEW_HEIGHT, GL_RGBA8);
    rbo.SetFormat(MODEL_VIEW_WIDTH, MODEL_VIEW_HEIGHT, GL_DEPTH_COMPONENT);

    fbo.Attach(GL_COLOR_ATTACHMENT0, fbTex);
    fbo.Attach(GL_DEPTH_ATTACHMENT, rbo);

    ModelTransform trans;

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        keyboardMgr.Capture();
        mouseMgr.Capture();
        winMgr.Capture();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        GL::RenderContext::SetClearColor(Vec4f(Vec3f(0.4f), 0.0f));
        GL::RenderContext::ClearColorAndDepth();

        if(ImGui::BeginMainMenuBar())
        {
            if(ImGui::BeginMenu("File"))
            {
                if(ImGui::MenuItem("Exit"))
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        {
            fbo.Bind();
            glViewport(0, 0, MODEL_VIEW_WIDTH, MODEL_VIEW_HEIGHT);

            GL::RenderContext::ClearColorAndDepth();

            model.SetWorld(
                Mat4f::Translate(trans.translate) *
                Mat4f::RotateX(trans.rotateX) *
                Mat4f::RotateZ(trans.rotateZ) *
                Mat4f::RotateY(trans.rotateY) *
                Mat4f::Scale(Vec3f(trans.scale)) *
                Mat4f::Translate(trans.preTranslate));
            model.Render();

            GL::RenderContext::DisableDepthTest();

            imm.DrawQuad({ -0.8f, -0.8f }, { 0.8f, 0.8f }, { 0.4f, 0.4f, 0.4f, 1.0f }, false);
            imm.DrawCircle({ 0.0f, 0.0f }, { 0.9f, 0.9f }, { 0.4f, 0.4f, 0.4f, 1.0f }, false);

            GL::RenderContext::EnableDepthTest();

            fbo.Unbind();

            int wW, wH;
            glfwGetFramebufferSize(window, &wW, &wH);
            glViewport(0, 0, wW, wH);
        }

        char viewerTitle[80];
        sprintf_s(viewerTitle, 80, "Viewer FPS %.1f###Viewer", ImGui::GetIO().Framerate);
        if(ImGui::Begin(viewerTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Image((ImTextureID)(size_t)(fbTex.GetHandle()), ImVec2(MODEL_VIEW_WIDTH, MODEL_VIEW_HEIGHT), { 0, 1 }, { 1, 0 });

            SetModelTransform(console, &trans);

            ImGui::End();
        }

        console.Display();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
