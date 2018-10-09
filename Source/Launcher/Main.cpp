#include <iostream>
#include <Atrc/Atrc.h>

using namespace std;
using namespace Atrc;

constexpr uint32_t SCR_W = 640;
constexpr uint32_t SCR_H = 480;

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

    const Vec3r eye = { -1.5, -5.0, 1.0 };
    const Vec3r dir = (Vec3r(0.0) - eye).Normalize();
    PerspectiveCamera camera(
        eye, dir, { 0.0, 0.0, 1.0 },
        Degr(60.0), SCR_ASPECT_RATIO);

    //============= Scene =============

    ColoredSky sky({ 0.4f, 0.7f, 0.9f }, { 1.0f, 1.0f, 1.0f });

    MatGeoEntity<Sphere> ground(NewRC<MetalMaterial>(Spectrum(0.4f, 0.8f, 0.4f), 0.35),
                                200, Transform::Translate({ 0.0, 0.0, -200 - 1.0 }));

    AGZ::Model::WavefrontObj arbShapeObj;
    AGZ::Model::WavefrontObjFile::LoadFromObjFile("./Assets/bunny.obj", &arbShapeObj);
    MatGeoEntity<TriangleBVH> arbShape(NewRC<MetalMaterial>(Spectrum(0.4f), 0.2),
                                       arbShapeObj.ToGeometryMeshGroup().submeshes["Default"],
                                       Transform::Translate({ 0.0, 0.0, -0.8 })
                                     * Transform::Scale(Vec3r(2.2 / 40)));

    Scene scene;
    scene.camera = &camera;
    scene.entities = { &ground, &arbShape, &sky };

    //============= Render Target =============

    RenderTarget<Color3f> renderTarget(SCR_W, SCR_H);

    //============= Rendering =============

    ParallelRenderer<JitteredSubareaRenderer> renderer(-1, 40);
    PathTracer integrator(10);

    cout << "Start rendering..." << endl;

    AGZ::Timer timer;
    renderer.Render(scene, integrator, renderTarget);
    auto deltaTime = timer.Milliseconds() / 1000.0;

    cout << "Complete rendering...Total time: " << deltaTime << "s." << endl;

    //============= Output =============

    AGZ::Tex::TextureFile::WriteRGBToPNG("./Build/Output.png", ToSavedImage(renderTarget, 2.2f));
}
