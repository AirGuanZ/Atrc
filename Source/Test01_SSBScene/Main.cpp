#include <iostream>
#include <Atrc/Atrc.h>

using namespace std;
using namespace Atrc;

constexpr uint32_t SCR_W = 640;
constexpr uint32_t SCR_H = 480;

constexpr Real SCR_ASPECT_RATIO = static_cast<Real>(SCR_W) / SCR_H;

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

RenderTarget<Color3b> ToSavedImage(const RenderTarget<Color3f> &origin, float gamma)
{
    return origin.Map([=](const Color3f &color)
    {
        return color.Map([=](float x)
        {
            return static_cast<uint8_t>(
                Clamp(Pow(ACESFilm(x), gamma),
                      0.0f, 1.0f) * 255);
        });
    });
}

void Render()
{
    //============= Camera =============

    const Vec3r eye = { -5.0, 0.0, 0.0 };
    const Vec3r dir = (Vec3r(0.0) - eye).Normalize();
    PerspectiveCamera camera(
        eye, dir, { 0.0, 0.0, 1.0 },
        Degr(90.0), SCR_ASPECT_RATIO);

    //============= Scene =============
    
    ColoredSky sky(Spectrum(0.0f), Spectrum(0.0f));

    MatGeoEntity<Transformer<Sphere>> ground(
        NewRC<DiffuseMaterial>(Spectrum(1.0f, 0.3f, 0.3f)),
        Transform::Translate({ 0.0, 0.0, -201.0 }), 200.0);

    MatGeoEntity<Transformer<Sphere>> centreSphere(
        NewRC<DiffuseMaterial>(Spectrum(0.4f, 0.7f, 0.2f)),
        TRANSFORM_IDENTITY, 1.0);
    
    MatGeoEntity<Transformer<Sphere>> leftSphere(
        NewRC<MetalMaterial>(Spectrum(0.9f), 0.0),
        Transform::Translate({ 0.0, 2.0, 0.0 }), 1.0);

    auto cubeTex = Tex2D<Color3b>(
        AGZ::Tex::TextureFile::LoadRGBFromFile(
        "./Assets/CubeTex.png")).Map(
            [](const Vec3<uint8_t> &vb)
    {
        return vb.Map([](uint8_t b) {return b / 255.0f; });
    });

    auto cubeMat = NewRC<TextureMultiplier<DiffuseMaterial, SamplingStrategy::Nearest>>(
                            cubeTex, Spectrum(0.7f));
    MatGeoEntity<Transformer<Cube>> rightDiffuseCube(
        cubeMat,
        Transform::Translate({ 0.0, -2.0, 0.123 })
      * Transform::Rotate({ 1.0, 1.1, 1.2 }, Degr(47)),
        0.7);

    MatGeoEntity<GeoDiffuseLight<Transformer<Sphere>>> lightSphere(
        NewRC<VoidMaterial>(), Spectrum(1.0f), Transform::Translate({ -1.5, 0.4, 0.5 }), 0.3);

    LightSelector lights({ &lightSphere });
    
    Scene scene;
    scene.camera   = &camera;
    scene.lightMgr = &lights;
    scene.entities = {
        &ground,
        &lightSphere,
        &centreSphere, &leftSphere, &rightDiffuseCube,
        &sky };

    //============= Render Target =============

    RenderTarget<Color3f> renderTarget(SCR_W, SCR_H);

    //============= Rendering =============

    ParallelRenderer<JitteredSubareaRenderer> renderer(6, 200);
    renderer.SetProgressPrinting(true);
    //PathTracerEx integrator(1, 10);
    PathTracerEx2 integrator(1, 10);
    //PathTracer integrator(10);

    cout << "Start rendering..." << endl;

    AGZ::Timer timer;
    renderer.Render(scene, integrator, renderTarget);
    auto deltaTime = timer.Milliseconds() / 1000.0;

    cout << "Complete rendering...Total time: " << deltaTime << "s." << endl;

    //============= Output =============

    AGZ::Tex::TextureFile::WriteRGBToPNG(
        "./Build/Output.png", ToSavedImage(renderTarget, 1.0f / 2.2f));
}

int main()
{
    try
    {
        Render();
    }
    catch(const std::exception &err)
    {
        cout << err.what() << endl;
    }
    catch(...)
    {
        cout << "Unknown exception..." << endl;
    }
}
