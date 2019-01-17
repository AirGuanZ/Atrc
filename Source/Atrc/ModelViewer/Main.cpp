#include <iostream>

#include <AGZUtils/Utils/Mesh.h>
#include <AGZUtils/Utils/Texture.h>

#include "GL.h"
#include "ModelRenderer.h"

using namespace std;

bool showLoadWindow = false;
void ShowLoadWindow()
{
    if(ImGui::Begin("Model loading", &showLoadWindow))
    {
        ImGui::Text("Test text");

        ImGui::End();
    }
}

int Run(GLFWwindow *window)
{
    glfwSwapInterval(1);
    
    using namespace AGZ::GraphicsAPI::GL;
    using namespace AGZ::GraphicsAPI;
    using namespace AGZ::Input;

    AGZ::ObjArena<> arena;

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

    Immediate imm;
    imm.Initialize({ 600.0f, 600.0f });

    // Model Renderer

    std::vector<ModelRenderer::Vertex> modelVtxData;
    std::vector<uint32_t> modelElemData;
    {
        AGZ::Mesh::WavefrontObj<float> obj;
        if(!obj.LoadFromFile("Scene/Asset/Test/Mitsuba.obj"))
        {
            cout << "Failed to load model data..." << endl;
            return -1;
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

    Texture2D modelTex(true);

    ModelRenderer model;
    model.Initialize();
    model.SetModelData(modelVtxData.data(), uint32_t(modelVtxData.size()), modelElemData.data(), uint32_t(modelElemData.size()));
    model.SetTexture(&modelTex);
    model.SetWorld(Mat4f::Translate({ 0.0f, -0.82f, 0.0f }) * Mat4f::RotateY(Deg(-90)));
    model.SetView(Mat4f::LookAt({ -4.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }));
    model.SetProj(Mat4f::Perspective(Deg(60.0f), 1.0f, 0.1f, 1000.0f));

    // 准备FBO

    FrameBuffer fbo(true);
    RenderBuffer rbo(true);
    Texture2D fbTex(true);
    fbTex.InitializeFormat(1, 300, 300, GL_RGBA8);
    rbo.SetFormat(300, 300, GL_DEPTH_COMPONENT);

    fbo.Attach(GL_COLOR_ATTACHMENT0, fbTex);
    fbo.Attach(GL_DEPTH_ATTACHMENT, rbo);
    {
        fbo.Bind();
        glViewport(0, 0, 300, 300);

        RenderContext::ClearColorAndDepth();

        imm.DrawQuad({ -0.8f, -0.8f }, { 0.8f, 0.8f }, { 0.4f, 0.4f, 0.4f, 1.0f }, false);
        imm.DrawCircle({ 0.0f, 0.0f }, { 0.9f, 0.9f }, { 0.4f, 0.4f, 0.4f, 1.0f }, false);

        RenderContext::EnableDepthTest();
        model.Render();

        fbo.Unbind();
        glViewport(0, 0, 640, 640);
    }

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        keyboardMgr.Capture();
        mouseMgr.Capture();
        winMgr.Capture();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        RenderContext::SetClearColor(Vec4f(Vec3f(0.4f), 0.0f));
        RenderContext::ClearColorAndDepth();

        if(ImGui::BeginMainMenuBar())
        {
            if(ImGui::BeginMenu("File"))
            {
                if(ImGui::MenuItem("Load"))
                    showLoadWindow = true;
                if(ImGui::MenuItem("Exit"))
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if(showLoadWindow)
            ShowLoadWindow();

        char viewerTitle[80];
        sprintf_s(viewerTitle, 80, "Viewer FPS %.1f###Viewer", ImGui::GetIO().Framerate);
        if(ImGui::Begin(viewerTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Image((ImTextureID)(size_t)(fbTex.GetHandle()), ImVec2(300, 300), { 0, 1 }, { 1, 0 });

            ImGui::End();
        }

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
