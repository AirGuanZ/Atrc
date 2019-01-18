#include <iostream>

#include <AGZUtils/Utils/Mesh.h>
#include <AGZUtils/Utils/Texture.h>

#include "Console.h"
#include "GL.h"
#include "ModelRenderer.h"
#include "TransformSequence.h"

using namespace std;
using AGZ::Str8;

constexpr int INIT_WIN_WIDTH = 1200;
constexpr int INIT_WIN_HEIGHT = 700;

constexpr int MODEL_VIEW_WIDTH = 300;
constexpr int MODEL_VIEW_HEIGHT = 300;

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

bool ShowModelLoadWindow(Console &console, ModelRenderer *model)
{
    AGZ_ASSERT(model);

    if(!ImGui::BeginPopupModal("LoadModelData", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        return false;

    if(ImGui::GetIO().KeysDown[AGZ::Input::KEY_ESCAPE])
    {
        ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        return false;
    }

    bool ret = false;

    static char buf[256] = "Scene/Asset/Test/Mitsuba.obj";
    ImGui::InputText("", buf, 256);

    ImGui::SameLine();
    if(ImGui::Button("load") || ImGui::GetIO().KeysDown[AGZ::Input::KEY_ENTER])
    {
        if(ReloadModelData(buf, model))
        {
            console.AddText(ConsoleText::Normal, std::string("Model loaded from ") + buf);
            ret = true;
            ImGui::CloseCurrentPopup();
        }
        else
            console.AddText(ConsoleText::Error, std::string("Failed to load model from ") + buf);
    }
    
    ImGui::EndPopup();
    return ret;
}

int Run(GLFWwindow *window)
{
    using namespace AGZ::GraphicsAPI;
    using namespace AGZ::Input;

    AGZ::ObjArena<> arena;

    Console console;
    TransformSequence transSeq;

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

    bool showLoadModelData = false;

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
            if(ImGui::BeginMenu("file"))
            {
                if(ImGui::MenuItem("load"))
                    showLoadModelData = true;
                if(ImGui::MenuItem("exit"))
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("transform"))
            {
                if(ImGui::MenuItem("reset"))
                {
                    transSeq.Clear();
                    console.AddText(ConsoleText::Normal, "Transform reset");
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if(showLoadModelData)
        {
            ImGui::OpenPopup("LoadModelData");
            showLoadModelData = false;
        }

        if(ShowModelLoadWindow(console, &model))
            transSeq.Clear();

        {
            fbo.Bind();
            glViewport(0, 0, MODEL_VIEW_WIDTH, MODEL_VIEW_HEIGHT);

            GL::RenderContext::ClearColorAndDepth();

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

        if(ImGui::Begin("Transform", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            transSeq.Display();
            model.SetWorld(transSeq.GetFinalTransformMatrix());
            ImGui::End();
        }

        char viewerTitle[80];
        sprintf_s(viewerTitle, 80, "Viewer FPS %.1f###Viewer", ImGui::GetIO().Framerate);
        if(ImGui::Begin(viewerTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Image(
                (ImTextureID)(size_t)(fbTex.GetHandle()), ImVec2(MODEL_VIEW_WIDTH, MODEL_VIEW_HEIGHT),
                { 0, 1 }, { 1, 0 });
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
