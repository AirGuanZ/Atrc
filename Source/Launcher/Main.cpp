#include <Atrc/Atrc.h>

using namespace std;
using namespace Atrc;

constexpr uint32_t SCR_W = 640;
constexpr uint32_t SCR_H = 480;

constexpr Real SCR_ASPECT_RATIO = static_cast<Real>(SCR_W) / SCR_H;

int main()
{
    PerspectiveCamera camera(
        { -5.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 },
        Degr(90.0), SCR_ASPECT_RATIO, 1.0);

    ColoredSky sky({ 0.4f, 0.7f, 0.9f }, { 1.0f, 1.0f, 1.0f });
    AmbientSphere ground(500.0, { 0.4f, 0.8f, 0.4f }, Transform::Translate({ 0.0, 0.0, -500.0 - 1.0 }));
    AmbientSphere centreBall(1.0, { 1.0f, 0.2f, 0.2f }, TRANSFORM_IDENTITY);

    Scene scene;
    scene.camera = &camera;
    scene.entities = { &sky, &ground, &centreBall };

    RenderTarget<Color3f> renderTarget(SCR_W, SCR_H);

    AmbientIntegrator integrator;
    // ParallelRenderer<Native1sppSubareaRenderer> renderer(-1);
    ParallelRenderer<JitteredSubareaRenderer> renderer(-1, 50);

    renderer.Render(scene, integrator, renderTarget);

    AGZ::Tex::TextureFile::WriteRGBToPNG(
        "Output/ColoredSky.png", AGZ::Tex::ClampedF2B(renderTarget));
}
