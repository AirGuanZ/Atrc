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

WStr boxOBJContent =
R"___(
# Blender v2.79 (sub 0) OBJ File: ''
# www.blender.org
mtllib untitled.mtl
o Cube
v -1.000000 1.000000 -1.000000
v 1.000000 1.000000 -1.000000
v 1.000000 -1.000000 -1.000000
v -1.000000 -1.000000 -1.000000
v -0.999999 1.000000 1.000000
v 1.000001 0.999999 1.000000
v 1.000000 -1.000000 1.000000
v -1.000000 -1.000000 1.000000
vn 0.0000 0.0000 -1.0000
vn 0.0000 0.0000 1.0000
vn 0.0000 1.0000 -0.0000
vn 1.0000 0.0000 -0.0000
vn -0.0000 -1.0000 -0.0000
vn -1.0000 0.0000 0.0000
usemtl Material
s off
f 2//1 4//1 1//1
f 8//2 6//2 5//2
f 5//3 2//3 1//3
f 6//4 3//4 2//4
f 3//5 8//5 4//5
f 1//6 8//6 5//6
f 2//1 3//1 4//1
f 8//2 7//2 6//2
f 5//3 6//3 2//3
f 6//4 7//4 3//4
f 3//5 7//5 8//5
f 1//6 4//6 8//6
)___";

int main()
{
    const Vec3r eye = { -5.0, 0.0, 0.0 };
    const Vec3r dir = (Vec3r(0.0) - eye).Normalize();
    PerspectiveCamera camera(
        eye, dir, { 0.0, 0.0, 1.0 },
        Degr(90.0), SCR_ASPECT_RATIO);

    ColoredSky sky({ 0.4f, 0.7f, 0.9f }, { 1.0f, 1.0f, 1.0f });
    MatGeoEntity<Sphere> ground(NewRC<DiffuseMaterial>(Spectrum(0.4f, 0.8f, 0.4f)),
                                200.0, Transform::Translate({ 0.0, 0.0, -201.0 }));
    MatGeoEntity<Sphere> centreBall(NewRC<DiffuseMaterial>(Spectrum(0.7f, 0.7f, 0.7f)),
                                    1.0, TRANSFORM_IDENTITY);
    MatGeoEntity<Sphere> leftMetalSphere(NewRC<MetalMaterial>(Spectrum(1.0f, 0.3f, 0.3f), 0.2),
                                         1.0, Transform::Translate({ 0.0, 2.0, 0.0 }));

    AGZ::Model::WavefrontObj boxOBJ;
    AGZ::Model::WavefrontObjFile::LoadFromMemory(boxOBJContent, &boxOBJ);
    auto boxMesh = boxOBJ.ToGeometryMeshGroup().submeshes["Cube"];
    TriangleBVH box(boxMesh, NewRC<DiffuseMaterial>(Spectrum(0.3f, 0.3f, 1.0f)),
                               Transform::Translate({ 0.0, -2.0, -0.1 }));

    Scene scene;
    scene.camera = &camera;
    scene.entities = { 
        &ground,
        &centreBall, &leftMetalSphere, &box,
        &sky };

    RenderTarget<Color3f> renderTarget(SCR_W, SCR_H);

    PathTracer integrator(10, 0.1);

    ParallelRenderer<JitteredSubareaRenderer> renderer(4, 20);
    renderer.Render(scene, integrator, renderTarget);

    AGZ::Tex::TextureFile::WriteRGBToPNG("Output.png", ToSavedImage(renderTarget, 2.2f));
}
