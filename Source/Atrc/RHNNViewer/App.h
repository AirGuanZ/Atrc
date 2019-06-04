#pragma once

#include <Atrc/RHNNViewer/GL.h>

class App
{
public:

    explicit App(GLFWwindow *window);

    void Run();

private:

    static constexpr int SHC = 9;
    static constexpr int COEFS_SIZE = 1024;

    void ShowMessage(const std::string &m);

    void RenderFrame();

    void ShowReconstructSettings();

    void LoadSceneCoefsFromFile(const std::filesystem::path &filename);

    void LoadLightCoefsFromFile(const std::filesystem::path &filename);

    void UpdateRotatedLightCoefs();

    void RenderReconstructedScene();

    struct ReconstructVertex
    {
        Vec2f iScreenPos;
        Vec2f iTexCoord;
    };

    GLFWwindow *window_;

    GL::Program reconstructProg_;

    GL::AttribVariable<Vec2f> attribScreenPos_;
    GL::AttribVariable<Vec2f> attribTexCoord_;
    GL::UniformVariable<Vec3f> uniformLightCoefs_[SHC];
    GL::UniformVariable<GL::Texture2DUnit> uniformSceneCoefs_[SHC];
    GL::UniformVariable<GLint> uniformWithAlbedo_;
    GL::UniformVariable<GL::Texture2DUnit> uniformAlbedo_;

    GL::VertexArray VAO_;

    GL::VertexBuffer<ReconstructVertex> reconstructVtxBuf_;
    std::unique_ptr<GL::Texture2D[]> reconstructSceneCoefs_;
    std::unique_ptr<Vec3f[]> reconstructLightCoefs_;

    bool withAlbedo_;
    std::unique_ptr<GL::Texture2D> albedo_;

    bool autoRotate_;
    float rotateDeg_;
    std::unique_ptr<Vec3f[]> rotatedLightCoefs_;

    AGZ::ObjArena<> arena_;

    AGZ::Input::KeyboardManager<AGZ::Input::GLFWKeyboardCapturer> keyboardMgr_;
    AGZ::Input::MouseManager   <AGZ::Input::GLFWMouseCapturer>    mouseMgr_;
    AGZ::Input::WindowManager  <AGZ::Input::GLFWWindowCapturer>   winMgr_;
};
