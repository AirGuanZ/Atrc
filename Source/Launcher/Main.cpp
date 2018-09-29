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
    
    Scene scene;
    scene.camera = &camera;
    scene.entities.push_back(&sky);

    RenderTarget<Color3f> renderTarget(SCR_W, SCR_H);

    AmbientIntegrator integrator;
    ParallelRenderer<Native1sppSubareaRenderer> renderer(-1);

    renderer.Render(scene, integrator, renderTarget);

    AGZ::Tex::TextureFile::WriteRGBToPNG(
        "Output/ColoredSky.png", AGZ::Tex::ClampedF2B(renderTarget));
}
