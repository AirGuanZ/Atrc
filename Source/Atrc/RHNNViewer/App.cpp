#include <Atrc/RHNNViewer/App.h>
#include <Atrc/RHNNViewer/ReconstructGLSL.h>
#include <Lib/cnpy/cnpy.h>
#include <Lib/ImFileBrowser/imfilebrowser.h>

App::App(GLFWwindow *window)
    : window_(window), autoRotate_(false), rotateDeg_(0)
{
    using namespace AGZ::Input;

    keyboardMgr_.GetCapturer().Initialize(window);
    mouseMgr_   .GetCapturer().Initialize(window);
    winMgr_     .GetCapturer().Initialize(window);

    auto &keyboard = keyboardMgr_.GetKeyboard();
    auto &mouse    = mouseMgr_.GetMouse();
    auto &win      = winMgr_.GetWindow();

    win.AttachHandler(arena_.Create<FramebufferSizeHandler>(
        [&](const FramebufferSize &param)
    {
        glViewport(0, 0, param.w, param.h);
    }));
    keyboard.AttachHandler(arena_.Create<KeyDownHandler>(
        [&](const KeyDown &param)
    {
        ImGui_ImplGlfw_KeyDown(param.key);
    }));
    keyboard.AttachHandler(arena_.Create<KeyUpHandler>(
        [&](const KeyUp &param)
    {
        ImGui_ImplGlfw_KeyUp(param.key);
    }));
    keyboard.AttachHandler(arena_.Create<CharEnterHandler>(
        [&](const CharEnter &param)
    {
        ImGui_ImplGlfw_Char(param.ch);
    }));
    mouse.AttachHandler(arena_.Create<MouseButtonDownHandler>(
        [&](const MouseButtonDown &param)
    {
        ImGui_ImplGlfw_MouseButtonDown(param.button);
    }));
    mouse.AttachHandler(arena_.Create<WheelScrollHandler>(
        [&](const WheelScroll &param)
    {
        ImGui_ImplGlfw_WheelScroll(param.offset);
    }));

    reconstructProg_ = GL::ProgramBuilder::BuildOnce(
        GL::VertexShader  ::FromMemory(RECONSTRUCT_VS_GLSL),
        GL::FragmentShader::FromMemory(RECONSTRUCT_FS_GLSL)
    );

    attribScreenPos_ = reconstructProg_.GetAttribVariable<Vec2f>("iScreenPos");
    attribTexCoord_ = reconstructProg_.GetAttribVariable<Vec2f>("iTexCoord");
    for(int i = 0; i < SHC; ++i)
    {
        auto si = std::to_string(i);
        uniformSceneCoefs_[i] = reconstructProg_.GetUniformVariableUnchecked<GL::Texture2DUnit>(("mSceneCoefs[" + si + "]").c_str());
        uniformLightCoefs_[i] = reconstructProg_.GetUniformVariableUnchecked<Vec3f>(("mLightCoefs[" + si + "]").c_str());
    }
    VAO_.InitializeHandle();
    VAO_.EnableAttrib(attribScreenPos_);
    VAO_.EnableAttrib(attribTexCoord_);

    reconstructVtxBuf_.InitializeHandle();
}

void App::Run()
{
    GL::RenderContext::SetClearColor({ 0, 0, 0, 1 });

    while(!glfwWindowShouldClose(window_))
    {
        glfwPollEvents();
        keyboardMgr_.Capture();
        mouseMgr_.Capture();
        winMgr_.Capture();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        GL::RenderContext::ClearColorAndDepth();

        RenderFrame();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window_);
    }
}

void App::ShowMessage(const std::string &m)
{
    std::cout << m << std::endl;
}

void App::RenderFrame()
{
    if(ImGui::Begin("reconstruct", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ShowReconstructSettings();
        ImGui::End();
    }

    RenderReconstructedScene();
}

void App::ShowReconstructSettings()
{
    constexpr auto FILE_BROWSER_FLAGS = ImGuiFileBrowserFlags_NoModal
                                      | ImGuiFileBrowserFlags_CloseOnEsc;
    static ImGui::FileBrowser sceneCoefFileBrowser(FILE_BROWSER_FLAGS);
    static ImGui::FileBrowser lightCoefFileBrowser(FILE_BROWSER_FLAGS);

    if(ImGui::Button("load scene"))
    {
        sceneCoefFileBrowser.SetTitle("load scene");
        sceneCoefFileBrowser.Open();
    }

    ImGui::SameLine();

    if(ImGui::Button("load light"))
    {
        lightCoefFileBrowser.SetTitle("load light");
        lightCoefFileBrowser.Open();
    }

    sceneCoefFileBrowser.Display();
    lightCoefFileBrowser.Display();

    if(sceneCoefFileBrowser.HasSelected())
    {
        auto sceneCoefFilename = sceneCoefFileBrowser.GetSelected();
        sceneCoefFileBrowser.ClearSelected();
        LoadSceneCoefsFromFile(sceneCoefFilename);
    }

    if(lightCoefFileBrowser.HasSelected())
    {
        auto lightCoefFilename = lightCoefFileBrowser.GetSelected();
        lightCoefFileBrowser.ClearSelected();
        LoadLightCoefsFromFile(lightCoefFilename);
    }

    ImGui::Checkbox("auto rot", &autoRotate_); ImGui::SameLine();

    if(ImGui::SliderFloat("rotate angle", &rotateDeg_, -360, 360, "%.3f deg"))
        UpdateRotatedLightCoefs();
    else if(!ImGui::IsItemActive() && autoRotate_)
    {
        rotateDeg_ += 0.7f;
        UpdateRotatedLightCoefs();
    }

    while(rotateDeg_ > 360)
        rotateDeg_ -= 360;
    while(rotateDeg_ < -360)
        rotateDeg_ += 360;
}

void App::LoadSceneCoefsFromFile(const std::filesystem::path &filename)
{
    std::vector<float> data(SHC * COEFS_SIZE * COEFS_SIZE);
    auto arr = cnpy::npz_load(filename.string())["coefs"];

    if(arr.shape.size() != 3 || arr.shape[0] != COEFS_SIZE || arr.shape[1] != COEFS_SIZE || arr.shape[2] != SHC)
    {
        ShowMessage("Invalid scene coef array shape in " + filename.string());
        return;
    }

    auto pData = arr.data<float>();
    if(arr.fortran_order)
        AGZ::Math::Permute<3>(pData, { COEFS_SIZE, COEFS_SIZE, SHC }, { 2, 1, 0 });

    for(int y = 0; y < COEFS_SIZE; ++y)
    {
        for(int x = 0; x < COEFS_SIZE; ++x)
        {
            for(int i = 0; i < SHC; ++i)
            {
                int idx = i * COEFS_SIZE * COEFS_SIZE + y * COEFS_SIZE + x;
                data[idx] = *pData++;
            }
        }
    }

    auto newTex = new GL::Texture2D[SHC];
    AGZ::ScopeGuard newTexGuard([&]{ delete[] newTex; });
    for(int i = 0; i < SHC; ++i)
    {
        auto initData = &data[i * COEFS_SIZE * COEFS_SIZE];
        newTex[i].InitializeHandle();
        newTex[i].InitializeFormatAndData(1, COEFS_SIZE, COEFS_SIZE, GL_R32F, initData);
    }

    reconstructSceneCoefs_.reset(newTex);
    newTexGuard.Dismiss();
}

void App::LoadLightCoefsFromFile(const std::filesystem::path &filename)
{
    std::array<float, SHC * 3> data = { 0 };
    auto arr = cnpy::npy_load(filename.string());

    if(arr.shape.size() != 2 || arr.shape[0] != SHC || arr.shape[1] != 3)
    {
        ShowMessage("Invalid light coef array shape in " + filename.string());
        return;
    }

    auto pData = arr.data<float>();
    if(arr.fortran_order)
        AGZ::Math::Permute<2>(pData, { SHC, 3 }, { 1, 0 });

    for(int i = 0; i < SHC; ++i)
    {
        for(int c = 0; c < 3; ++c)
            data[i * 3 + c] = *pData++;
    }

    auto newLights = new Vec3f[SHC];
    AGZ::ScopeGuard newLightsGuard([&] { delete[] newLights; });
    for(int i = 0; i < SHC; ++i)
        newLights[i] = Vec3f(&data[i * 3]);

    reconstructLightCoefs_.reset(newLights);
    newLightsGuard.Dismiss();

    UpdateRotatedLightCoefs();
}

void App::UpdateRotatedLightCoefs()
{
    if(!reconstructLightCoefs_)
    {
        rotatedLightCoefs_ = nullptr;
        return;
    }

    auto *newData = new Vec3f[SHC];
    AGZ::ScopeGuard newDataGuard([&] { delete[] newData; });

    std::vector<float> channels[3];
    for(int c = 0; c < 3; ++c)
    {
        channels[c].resize(SHC);
        for(int i = 0; i < SHC; ++i)
            channels[c][i] = reconstructLightCoefs_[i][c];
    }

    auto rotateMat = AGZ::Math::RM_Mat3f::RotateY(Deg(rotateDeg_));

    auto rot = [&](int offset, void(*f)(const AGZ::Math::RM_Mat3f&, float*))
    {
        for(int c = 0; c < 3; ++c)
            f(rotateMat, &channels[c][offset]);
    };
    rot(0, &AGZ::Math::SH::RotateSH_L0<float>);
    rot(1, &AGZ::Math::SH::RotateSH_L1<float>);
    rot(4, &AGZ::Math::SH::RotateSH_L2<float>);

    for(int c = 0; c < 3; ++c)
    {
        for(int i = 0; i < SHC; ++i)
            newData[i][c] = channels[c][i];
    }

    rotatedLightCoefs_.reset(newData);
    newDataGuard.Dismiss();
}

void App::RenderReconstructedScene()
{
    if(!reconstructSceneCoefs_ || !rotatedLightCoefs_)
        return;

    int fbW, fbH;
    glfwGetFramebufferSize(window_, &fbW, &fbH);

    float imgPixelSize = std::min(fbW, fbH) - 50.0f;
    float imgRatioW = imgPixelSize / fbW, imgRatioH = imgPixelSize / fbH;

    ReconstructVertex vtxData[] =
    {
        { { -imgRatioW, -imgRatioH }, { 0, 1 } },
        { { -imgRatioW,  imgRatioH }, { 0, 0 } },
        { {  imgRatioW,  imgRatioH }, { 1, 0 } },

        { { -imgRatioW, -imgRatioH }, { 0, 1 } },
        { {  imgRatioW,  imgRatioH }, { 1, 0 } },
        { {  imgRatioW, -imgRatioH }, { 1, 1 } }
    };
    reconstructVtxBuf_.ReinitializeData(vtxData, 6, GL_STATIC_DRAW);

    reconstructProg_.Bind();
    VAO_.BindVertexBufferToAttrib(attribScreenPos_, reconstructVtxBuf_, &ReconstructVertex::iScreenPos, 0);
    VAO_.BindVertexBufferToAttrib(attribTexCoord_, reconstructVtxBuf_, &ReconstructVertex::iTexCoord, 1);
    VAO_.Bind();

    for(int i = 0; i < SHC; ++i)
        uniformLightCoefs_[i].BindValue(rotatedLightCoefs_[i]);
    for(int i = 0; i < SHC; ++i)
    {
        uniformSceneCoefs_[i].BindValue({ GLuint(i) });
        reconstructSceneCoefs_[i].Bind(i);
    }

    GL::RenderContext::DrawVertices(GL_TRIANGLES, 0, 6);

    VAO_.Unbind();
    reconstructProg_.Unbind();
}
