#include <iostream>
#include <Atrc/Atrc.h>
#include "Atrc/Material/TextureMultiplier.h"

using namespace std;
using namespace Atrc;

constexpr uint32_t SCR_W = 1920;
constexpr uint32_t SCR_H = 1080;

constexpr Real SCR_ASPECT_RATIO = static_cast<Real>(SCR_W) / SCR_H;

RenderTarget<Color3b> ToSavedImage(const RenderTarget<Color3f> &origin, float gamma)
{
    float index = 1.0f / gamma;
    return origin.Map([=](const Color3f &c)
    {
        return c.Map([=](float f)
        {
            return static_cast<uint8_t>(
                Clamp(Pow(f, index), 0.0f, 1.0f) * 255);
        });
    });
}

int main()
{
    //============= Camera =============

    const Vec3r eye = { -5.0, 0.0, 0.0 };
    const Vec3r dir = (Vec3r(0.0) - eye).Normalize();
    PerspectiveCamera camera(
        eye, dir, { 0.0, 0.0, 1.0 },
        Degr(90.0), SCR_ASPECT_RATIO);

    //============= Scene =============

    ColoredSky sky({ 0.4f, 0.7f, 0.9f }, { 1.0f, 1.0f, 1.0f });

    MatGeoEntity<Transformer<Sphere>> ground(
        NewRC<DiffuseMaterial>(Spectrum(0.3f, 0.3f, 1.0f)),
        Transform::Translate({ 0.0, 0.0, -201.0 }), 200.0);

    MatGeoEntity<Transformer<Sphere>> centreBall(
        NewRC<DiffuseMaterial>(Spectrum(0.4f, 0.7f, 0.2f)),
        TRANSFORM_IDENTITY, 1.0);

    MatGeoEntity<Transformer<Sphere>> leftMetalSphere(
        NewRC<MetalMaterial>(Spectrum(1.0f, 0.3f, 0.3f), 0.2),
        Transform::Translate({ 0.0, 2.0, 0.0 }), 1.0);

    auto cubeTex = Tex2D<Color3b>(
        AGZ::Tex::TextureFile::LoadRGBFromFile(
        "./Assets/CubeTex.png")).Map(
            [](const Vec3<uint8_t> &vb)
    {
        return vb.Map([](uint8_t b) {return b / 255.0f; });
    });

    MatGeoEntity<Transformer<Cube>> rightDiffuseCube(
        NewRC<TextureMultiplier<DiffuseMaterial>>(
            cubeTex, Spectrum(0.8f)),
        Transform::Translate({ 0.0, -2.0, 0.123 })
      * Transform::Rotate({ 1.0, 1.1, 1.2 }, Degr(47)),
        0.7);

    Scene scene;
    scene.camera = &camera;
    scene.entities = {
        &ground,
        &centreBall, &leftMetalSphere, &rightDiffuseCube,
        &sky };

    //============= Render Target =============

    RenderTarget<Color3f> renderTarget(SCR_W, SCR_H);

    //============= Rendering =============

    ParallelRenderer<JitteredSubareaRenderer> renderer(-1, 2000);
    PathTracer integrator(20);

    cout << "Start rendering..." << endl;

    AGZ::Timer timer;
    renderer.Render(scene, integrator, renderTarget);
    auto deltaTime = timer.Milliseconds() / 1000.0;

    cout << "Complete rendering...Total time: " << deltaTime << "s." << endl;

    //============= Output =============

    AGZ::Tex::TextureFile::WriteRGBToPNG(
        "./Build/Output.png", ToSavedImage(renderTarget, 2.2f));
}
