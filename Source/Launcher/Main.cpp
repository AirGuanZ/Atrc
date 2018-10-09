#include <iostream>
#include <Atrc/Atrc.h>

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

void Run()
{
    //============= Camera =============

    const Vec3r eye = { -2.0, -4.0, 1.0 };
    const Vec3r dir = (Vec3r(0.0) - eye).Normalize();
    PerspectiveCamera camera(
        eye, dir, { 0.0, 0.0, 1.0 },
        Degr(60.0), SCR_ASPECT_RATIO);

    //============= Scene =============

    ColoredSky sky({ 0.4f, 0.7f, 0.9f }, { 1.0f, 1.0f, 1.0f });

    MatGeoEntity<Sphere> ground(
        NewRC<DiffuseMaterial>(Spectrum(0.4f, 0.8f, 0.4f)),
        200, Transform::Translate({ 0.0, 0.0, -200 - 1.0 }));

    AGZ::Model::WavefrontObj dragonObj;
    AGZ::Model::WavefrontObjFile::LoadFromObjFile("./Assets/dragon_vrip.obj", &dragonObj);
    MatGeoEntity<TriangleBVH> dragon(
        NewRC<DiffuseMaterial>(Spectrum(0.4f)),
        dragonObj.ToGeometryMeshGroup().submeshes["Default"],
        Transform::Translate({ 0.0, 0.0, -0.67 })
      * Transform::Scale(Vec3r(2.2 / 50)));
    dragonObj.Clear();

    AGZ::Model::WavefrontObj bunnyObj;
    AGZ::Model::WavefrontObjFile::LoadFromObjFile("./Assets/bunny.obj", &bunnyObj);
    MatGeoEntity<TriangleBVH> bunny(
        NewRC<MetalMaterial>(Spectrum(0.7f), 0.1),
        bunnyObj.ToGeometryMeshGroup().submeshes["Default"],
        Transform::Translate({ 2.5, 1.0, -0.85 })
      * Transform::RotateZ(Degr(90))
      * Transform::Scale(Vec3r(2.2 / 60)));
    bunnyObj.Clear();

    Scene scene;
    scene.camera = &camera;
    scene.entities = { &ground, &dragon, &bunny, &sky };

    //============= Render Target =============

    RenderTarget<Color3f> renderTarget(SCR_W, SCR_H);

    //============= Rendering =============

    ParallelRenderer<JitteredSubareaRenderer> renderer(6, 100);
    PathTracer integrator(10);

    cout << "Start rendering..." << endl;

    AGZ::Timer timer;
    renderer.Render(scene, integrator, renderTarget);
    auto deltaTime = timer.Milliseconds() / 1000.0;

    cout << "Complete rendering...Total time: " << deltaTime << "s." << endl;

    //============= Output =============

    AGZ::Tex::TextureFile::WriteRGBToPNG("./Build/Output.png", ToSavedImage(renderTarget, 2.2f));
}

int main()
{
    try
    {
        Run();
    }
    catch(const std::exception &err)
    {
        cout << err.what() << endl;
    }
    catch(...)
    {
        cout << "An unknown exception occurred. Exiting..." << endl;
    }
}
