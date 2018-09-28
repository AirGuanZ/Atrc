#define AGZ_TEXTURE_FILE_IMPL
#include <Utils.h>

#include <Camera/PerspectiveCamera.h>
#include <Entity/GeometryEntity.h>
#include <Integrator/WhittedRayTracer.h>
#include <Light/PointLight.h>
#include <Material/PureColor.h>
#include <Math/Geometry/Sphere.h>
#include <Renderer/Native1sppRenderer.h>
#include <Renderer/NativeParallelRenderer.h>
#include <Sample/BufferedSampleSequence.h>
#include <Sample/GlobalSampleGenerator.h>

using namespace Atrc;

constexpr int SCR_W = 640, SCR_H = 480;
constexpr Real SCR_ASPECT = static_cast<Real>(SCR_W) / SCR_H;

constexpr Vec3r CAMERA_EYE(-3.0, 0.0, 1.0), CAMERA_DIR(3.0, 0.0, -1.0), CAMERA_UP(0.0, 0.0, 1.0);

int main()
{
    PerspectiveCamera camera(
        CAMERA_EYE, CAMERA_DIR, CAMERA_UP,
        Deg2Rad(Degr(60.0)), SCR_ASPECT, 1.0);

    PureColorMaterial matRed({ 1.0f, 0.4f, 0.4f }),
        matGreen({ 0.4f, 1.0f, 0.4f }),
        matBlue({ 0.4f, 0.4f, 1.0f }),
        matWhite({ 0.4f, 0.4f, 0.4f });

    Sphere sphGround(1e5), sphRed(0.2), sphGreen(0.4), sphBlue(0.6);

    GeometryEntity entGround(&sphGround, &matWhite, Transform(Mat4r::Translate({ 0.0, 0.0, -1e5 - 5.0 }))),
        entRed(&sphRed, &matRed, Transform(Mat4r::Translate({ -0.8, 0.0, 0.2 }))),
        entGreen(&sphGreen, &matGreen, Transform(Mat4r::Translate({ 0.0, 0.3, 0.0 }))),
        entBlue(&sphBlue, &matBlue, Transform(Mat4r::Translate({ 0.1, -0.4, 0.2 })));

    PointLight lightSkyblue({ 4.0f, 6.0f, 4.0f }, { -1.5, 0.0, 2.0 });

    SceneView scene;
    scene.camera = &camera;
    scene.entities = { &entGround, &entRed, &entGreen, &entBlue };
    scene.lights = { &lightSkyblue };

    RenderTarget<Color3f> renderTarget(SCR_W, SCR_H);

    GlobalSampleGenerator2D globalSamGen;
    SampleGenerator2Sequence<GlobalSampleGenerator2D> globalSamSeq(globalSamGen);
    BufferedSampleSequence<Sample2D> samGen(globalSamSeq);

    WhittedRayTracer integrator(globalSamSeq);
    NativeParallelRenderer<Native1sppSubareaRenderer> renderer;
    renderer.Render(scene, integrator, renderTarget);

    AGZ::Tex::TextureFile::WriteRGBToPNG(
        L"ExampleOutput_WhittedIntegrator.png", AGZ::Tex::ClampedF2B(renderTarget));
}
