#include <iostream>

#include <Atrc/Atrc.h>
#include <Utils.h>

using namespace Atrc;
using namespace std;

// See https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
float ACESFilm(float x)
{
    constexpr float a = 2.51f;
    constexpr float b = 0.03f;
    constexpr float c = 2.43f;
    constexpr float d = 0.59f;
    constexpr float e = 0.14f;
    return Clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
}

AGZ::Tex::Texture2D<Color3b> ToSavedImage(const RenderTarget &origin, float gamma)
{
    return origin.Map([=](const Color3f &color)
    {
        return color.Map([=](float x)
        {
            return static_cast<uint8_t>(Clamp(Pow(ACESFilm(x), gamma), 0.0f, 1.0f) * 255);
        });
    });
}

int main()
{
    constexpr uint32_t SCR_W = 640;
    constexpr uint32_t SCR_H = 480;
    constexpr Real SCR_ASPECT_RATIO = static_cast<Real>(SCR_W) / SCR_H;

    //============= Camera =============

    const Vec3 eye = { -5.0, 0.0, 0.0 };
    const Vec3 dir = Vec3(0.0) - eye;
    PerspectiveCamera camera(eye, dir, { 0.0, 0.0, 1.0 }, Deg(60.0), SCR_ASPECT_RATIO);

    //============= Scene =============

    Sphere sph(Transform::Translate(0.0, 0.0, -201.0), 200.0);
    GeometryEntity ground(&sph, &STATIC_BLACK_MATERIAL);

    Sphere sph2(Transform(), 1.0);
    GeometryEntity sphere(&sph2, &STATIC_BLACK_MATERIAL);

    Scene scene;
    scene.camera = &camera;
    scene.entities_ = { &ground, &sphere };

    //============= Render Target =============

    RenderTarget renderTarget(SCR_W, SCR_H);

    //============= Renderer & Integrator =============

    JitteredSubareaRenderer subareaRenderer(100);

    ParallelRenderer renderer;
    renderer.SetProgressPrinting(true);

    PureColorIntegrator integrator(SPECTRUM::WHITE, SPECTRUM::BLACK);

    //============= Rendering =============

    cout << "Start rendering..." << endl;

    AGZ::Timer timer;
    renderer.Render(subareaRenderer, scene, integrator, renderTarget);
    auto deltaTime = timer.Milliseconds() / 1000.0;

    cout << "Complete rendering...Total time: " << deltaTime << "s." << endl;

    //============= Output =============

    AGZ::Tex::TextureFile::WriteRGBToPNG("./Build/Output.png", ToSavedImage(renderTarget, 2.2f));
}
