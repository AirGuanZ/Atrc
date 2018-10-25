#include <iostream>

#include <Atrc/Atrc.h>
#include <Utils.h>

using namespace AGZ;
using namespace Atrc;
using namespace std;

// See https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
float ACESFilm(float x)
{
    /*constexpr float a = 2.51f;
    constexpr float b = 0.03f;
    constexpr float c = 2.43f;
    constexpr float d = 0.59f;
    constexpr float e = 0.14f;
    return Clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);*/
    return x;
}

Tex::Texture2D<Color3b> ToSavedImage(const RenderTarget &origin, float gamma)
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
    constexpr uint32_t SCR_W = 1200;
    constexpr uint32_t SCR_H = 675;
    constexpr Real SCR_ASPECT_RATIO = static_cast<Real>(SCR_W) / SCR_H;

    //============= Camera =============

    const Vec3 eye = { -5.0, 0.0, 0.0 };
    const Vec3 dir = Vec3(0.0, 0.0, 0.0) - eye;
    PerspectiveCamera camera(eye, dir, { 0.0, 0.0, 1.0 }, Deg(90.0), SCR_ASPECT_RATIO);

    //============= Scene =============

    Sphere sph(Transform::Translate(0.0, 0.0, -201.0), 200.0);
    DiffuseMaterial groundMat(Spectrum(0.4f, 0.8f, 0.4f));
    GeometricEntity ground(&sph, &groundMat);

    Sphere sph2(Transform::Translate(0.0, 0.0, 0.0), 1.0);
    DiffuseMaterial medMat(Spectrum(0.7f));
    GeometricEntity medSph(&sph2, &medMat);

    Sphere sph3(Transform::Translate(0.0, 2.0, 0.0), 1.0);
    FresnelSpecular leftMat(Spectrum(0.8f), MakeRC<FresnelDielectric>(1.0f, 1.5f));
    GeometricEntity leftSph(&sph3, &leftMat);

    Sphere sph4(Transform::Translate(0.0, 0.5, 0.7), 0.25);
    GeometricDiffuseLight upLight(&sph4, Spectrum(25.0f));

    Model::WavefrontObj dragonObj;
    Model::WavefrontObjFile::LoadFromObjFile("./Assets/dragon_vrip.obj", &dragonObj);
    auto dragonBVHCore = MakeRC<TriangleBVHCore>(dragonObj.ToGeometryMeshGroup().submeshes["Default"]);
    dragonObj.Clear();
    TriangleBVH dragonBVH(Transform::Translate(-1.5, -2.8, -0.6) * Transform::RotateZ(Deg(-90.0)) * Transform::Scale(2.2 / 40), dragonBVHCore);
    IdealMirror dragonMat(Spectrum(0.6f), MakeRC<FresnelDielectric>(1.0f, 0.01f));
    GeometricEntity dragon(&dragonBVH, &dragonMat);

    SkyLight sky(Spectrum(0.4f, 0.7f, 0.9f), Spectrum(1.0f));

    Scene scene;
    scene.camera    = &camera;
    scene.lights_   = { &sky };
    scene.entities_ = { &ground, &medSph, &leftSph, &dragon };

    for(auto ent : scene.GetEntities())
    {
        auto L = ent->AsLight();
        if(L)
            scene.lights_.push_back(L);
    }

    sky.PreprocessScene(scene);

    //============= Render Target =============

    RenderTarget renderTarget(SCR_W, SCR_H);

    //============= Renderer & Integrator =============

    JitteredSubareaRenderer subareaRenderer(10);

    ParallelRenderer renderer(6);
    //SerialRenderer renderer;
    renderer.SetProgressPrinting(true);

    //PureColorIntegrator integrator(SPECTRUM::BLACK, SPECTRUM::WHITE);
    PathTracer integrator(10);

    //============= Rendering =============

    cout << "Start rendering..." << endl;

    Timer timer;
    renderer.Render(subareaRenderer, scene, integrator, renderTarget);
    auto deltaTime = timer.Milliseconds() / 1000.0;

    cout << "Complete rendering...Total time: " << deltaTime << "s." << endl;

    //============= Output =============

    Tex::TextureFile::WriteRGBToPNG("./Build/Output.png", ToSavedImage(renderTarget, 1 / 2.2f));
}
