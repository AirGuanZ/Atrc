#include <Camera/PerspectiveCamera.h>
#include <Entity/GeometryEntity.h>
#include <Integrator/AmbientIntegrator.h>
#include <Material/PureColor.h>
#include <Math/Geometry/Sphere.h>

using namespace Atrc;

constexpr int SCR_W = 640, SCR_H = 480;
constexpr Real SCR_ASPECT = static_cast<Real>(SCR_W) / SCR_H;

int main()
{
    PerspectiveCamera camera(
        { -6.0, 4.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 },
        Deg2Rad(Degr(60.0)), SCR_ASPECT, 1.0);

    PureColorMaterial matRed(SPECTRUM::RED),
                      matGreen(SPECTRUM::GREEN),
                      matBlue(SPECTRUM::BLUE),
                      matWhite(SPECTRUM::WHITE);

    Sphere sphGround(1e5), sphRed(0.2), sphGreen(0.4), sphBlue(0.6);

    GeometryEntity entGround(&sphGround, &matWhite, Transform(Mat4r::Translate({ 0.0, 0.0, -1e5 - 5.0 }))),
                   entRed   (&sphRed,    &matRed,   Transform(Mat4r::Translate({ -0.8, 0.0, 0.0 }))),
                   entGreen (&sphGreen,  &matGreen, Transform(Mat4r::Translate({ 0.0, 0.3, 0.0 }))),
                   entBlue  (&sphBlue,   &matBlue,  Transform(Mat4r::Translate({ 0.1, -0.4, 0.2 })));

    SceneView scene;
    scene.camera = &camera;
    scene.entities = { &entGround, &entRed, &entGreen, &entBlue };

    // TODO
}
