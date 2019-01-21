#include <iostream>

#include <AGZUtils/Utils/Mesh.h>
#include <AGZUtils/Utils/Texture.h>

#include "Camera.h"
#include "Console.h"
#include "GL.h"
#include "Global.h"
#include "ModelDataManager.h"
#include "ModelRenderer.h"
#include "TransformSequence.h"

using namespace std;
using AGZ::Str8;

constexpr int INIT_WIN_WIDTH = 1400;
constexpr int INIT_WIN_HEIGHT = 900;

bool CreateModelFromSelectedData(Console &console, const ModelDataManager &dataMgr, ModelRenderer *model)
{
    AGZ_ASSERT(model);

    std::vector<ModelRenderer::Vertex> modelVtxData;
    std::vector<uint32_t> modelElemData;

    auto meshGrpData = dataMgr.GetSelectedMeshGroup();
    if(!meshGrpData)
    {
        console.AddText(ConsoleText::Error, "No model data is selected");
        return false;
    }

    for(auto &it : meshGrpData->meshGroup.submeshes)
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

    model->SetModelData(
        modelVtxData.data(), uint32_t(modelVtxData.size()),
        modelElemData.data(), uint32_t(modelElemData.size()));

    console.AddText(ConsoleText::Normal, "Using model data: " + meshGrpData->name);
    return true;
}

int Run(GLFWwindow *window)
{
    using namespace AGZ::GraphicsAPI;
    using namespace AGZ::Input;

    AGZ::ObjArena<> arena;

    Camera camera("default");
    Console console;
    ModelDataManager modelDataMgr;
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
        auto &global = Global::GetInstance();
        global.framebufferWidth  = param.w;
        global.framebufferHeight = param.h;
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
    model.SetProj(Mat4f::Perspective(Deg(60.0f), float(INIT_WIN_WIDTH) / INIT_WIN_HEIGHT, 0.1f, 1000.0f));

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        keyboardMgr.Capture();
        mouseMgr.Capture();
        winMgr.Capture();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        bool showLoadModelData = false;

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

            ImGui::EndMainMenuBar();
        }

        if(showLoadModelData)
        {
            if(CreateModelFromSelectedData(console, modelDataMgr, &model))
                transSeq.Clear();
        }

        {
            float fbW = static_cast<float>(Global::GetInstance().framebufferWidth);
            float fbH = static_cast<float>(Global::GetInstance().framebufferHeight);
            float posX = 40;
            float posY = ImGui::GetFrameHeight() + posX * (fbH / fbW);
            ImGui::SetNextWindowPos(ImVec2(posX, posY), ImGuiCond_FirstUseEver);
        }
        if(ImGui::Begin("Scene Manager", nullptr, ImVec2(400, 700)))
        {
            if(ImGui::BeginTabBar("scene manager tab"))
            {
                if(ImGui::BeginTabItem("model"))
                {
                    modelDataMgr.Display(console);
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("property"))
                {
                    if(ImGui::CollapsingHeader("transform"))
                        transSeq.Display();
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("camera"))
                {
                    camera.Display();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::End();
        }

        console.Display();

        GL::RenderContext::SetClearColor(Vec4f(Vec3f(0.4f), 0.0f));
        GL::RenderContext::ClearColorAndDepth();
        GL::RenderContext::EnableDepthTest();

        model.SetView(camera.GetViewMatrix());
        model.SetProj(camera.GetProjMatrix());
        model.SetWorld(transSeq.GetFinalTransformMatrix());
        model.Render();

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
    glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
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

    {
        auto &global = Global::GetInstance();
        glfwGetFramebufferSize(window, &global.framebufferWidth, &global.framebufferHeight);
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
