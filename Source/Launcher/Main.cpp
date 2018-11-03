#include <iostream>

#include <Atrc/Atrc.h>
#include <Utils.h>

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

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

Texture2D<Color3b> ToSavedImage(const RenderTarget &origin, float gamma)
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
#if defined(_MSC_VER) && defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)
                 | _CRTDBG_LEAK_CHECK_DF);
#endif

    constexpr uint32_t SCR_W = 1200;
    constexpr uint32_t SCR_H = 675;
    constexpr Real SCR_ASPECT_RATIO = static_cast<Real>(SCR_W) / SCR_H;

    //============= Camera =============

    const Vec3 eye = Vec3(-7.0, 0.0, 0.0);
    const Vec3 dir = Vec3(0.0, -0.3, 0.0) - eye;
    PerspectiveCamera camera(eye, dir, { 0.0, 0.0, 1.0 }, Deg(40.0), SCR_ASPECT_RATIO);

    //============= Scene =============

    Sphere sph(Transform::Translate(0.0, 0.0, -201.0), 200.0);
    DiffuseMaterial groundMat(Spectrum(0.4f, 0.8f, 0.4f));
    GeometricEntity ground(&sph, &groundMat);

    Model::WavefrontObj medObj;
    Model::WavefrontObjFile::LoadFromObjFile("./Assets/test.obj", &medObj);
    auto medBVHCore = MakeRC<TriangleBVHCore>(medObj.ToGeometryMeshGroup().submeshes["Default"]);
    medObj.Clear();
    TriangleBVH medBVH(Transform::Translate(0.0, 0.25, -1.0) * Transform::RotateZ(Deg(-90)) * Transform::Scale(1.0 / 10000), medBVHCore);

    Sphere sph2(Transform::Translate(0.0, 0.0, 0.0), 1.0);
    Metal medMat(Spectrum(0.9f, 0.4f, 0.2f), Spectrum(0.01f), Spectrum(0.1f), 0.01);
    GeometricEntity medModel(&medBVH, &medMat);

    Sphere sph3(Transform::Translate(0.0, 2.0, -0.3), 0.7);
    Metal leftMat(Spectrum(0.5f), Spectrum(0.1f), Spectrum(0.1f), 0.003);
    GeometricEntity leftSph(&sph3, &leftMat);

    Model::WavefrontObj dragonObj;
    Model::WavefrontObjFile::LoadFromObjFile("./Assets/bun_zipper.obj", &dragonObj);
    auto dragonBVHCore = MakeRC<TriangleBVHCore>(dragonObj.ToGeometryMeshGroup().submeshes["Default"]);
    dragonObj.Clear();
    TriangleBVH dragonBVH(Transform::Translate(0.0, -0.4, -0.8) * Transform::RotateZ(Deg(-90)) * Transform::Scale(2.2 / 50), dragonBVHCore);
    Metal dragonMat(Spectrum(0.9f, 0.4f, 0.2f), Spectrum(0.1f), Spectrum(0.05f), 0.04);
    GeometricEntity dragon(&dragonBVH, &dragonMat);

    Texture2D<Spectrum> cubeTex = Texture2D<Spectrum>(
    TextureFile::LoadRGBFromFile("./Assets/CubeTex.png").Map(
    [](const Color3b &c) { return c.Map([](uint8_t b) { return b / 255.0f; }); }));

    Cube cube(Transform::Translate(0.0, -2.0, 0.123) * Transform::Rotate({ 1.0, 1.1, 1.2 }, Deg(47)), 1.4);
    DiffuseMaterial cubeDiffuse(Spectrum(0.2f, 0.4f, 0.8f));
    TextureScaler<Atrc::LinearSampler> cubeMat(&cubeTex, &cubeDiffuse);
    GeometricEntity rightCube(&cube, &cubeMat);

    SkyLight sky(Spectrum(0.4f, 0.7f, 0.9f), Spectrum(1.0f));
	//SkyLight sky(Spectrum(0.01f, 0.0f, 0.0f), Spectrum(0.01f, 0.0f, 0.0f));

    Model::WavefrontObj ringObj;
    Model::WavefrontObjFile::LoadFromObjFile("./Assets/ring.obj", &ringObj);
    auto ringBVHCore = MakeRC<TriangleBVHCore>(ringObj.ToGeometryMeshGroup().submeshes["Default"]);
    ringObj.Clear();
    TriangleBVH ringBVH(Transform::Translate(0.0, 0.78, -0.37) * Transform::RotateZ(Deg(90)) * Transform::Scale(1.0 / 20.0), ringBVHCore);

    Sphere sph4(Transform::Translate(0.0, 0.78, -0.4), 0.05);
    Cube cubeL(Transform::Translate(2.0, 1.0, 1.8) * Transform::Rotate({ 1.0, 1.1, 1.2 }, Deg(47)), 0.7);
    GeometricDiffuseLight sphLight(&sph4, Spectrum(13.0f));

    std::vector<const Entity*> entities = { /*&rightCube, &leftSph,*/ &dragon, /*&medModel,*/ &ground, /*&sphLight*/ };

    Scene scene;
    scene.camera    = &camera;
    scene.lights_   = { &sky };

    for(auto ent : entities)
    {
        auto L = ent->AsLight();
        if(L)
            scene.lights_.push_back(L);
    }

    BVH bvh(entities);
	scene.entities_ = { &bvh };

    sky.PreprocessScene(scene);
    sphLight.AsLight()->PreprocessScene(scene);

    //============= Render Target =============

    RenderTarget renderTarget(SCR_W, SCR_H);

    //============= Renderer & Integrator =============

    JitteredSubareaRenderer subareaRenderer(1);

    ParallelRenderer renderer(6);
    //SerialRenderer renderer;
    renderer.EnableProgressPrinting(true);

    //PureColorIntegrator integrator(SPECTRUM::BLACK, SPECTRUM::WHITE);
    PathTracer integrator(10);

    //============= Rendering =============

    cout << "Start rendering..." << endl;

    Timer timer;
    renderer.Render(subareaRenderer, scene, integrator, renderTarget);
    auto deltaTime = timer.Milliseconds() / 1000.0;

    cout << "Complete rendering...Total time: " << deltaTime << "s." << endl;

    //============= Output =============

    TextureFile::WriteRGBToPNG("./Build/Output.png", ToSavedImage(renderTarget, 1 / 2.2f));
}
