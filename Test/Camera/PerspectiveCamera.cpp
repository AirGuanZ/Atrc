#include <memory>
#include <Camera/PerspectiveCamera.h>

#include "../Catch.hpp"

using namespace AGZ;
using namespace Atrc;

TEST_CASE("PerspectiveCamera", "Camera")
{
    std::unique_ptr<Camera> camera = std::make_unique<PerspectiveCamera>(
        Vec3r(0.0, 0.0, 0.0), Vec3r(1.0, 0.0, 0.0), Vec3r(0.0, 0.0, 1.0),
        AsRad(Degr(60.0)), 640.0 / 480.0, 1.0);

    REQUIRE(ApproxEq(camera->Generate({ 0.0, 0.0 }).origin, Vec3r(1.0, 0.0, 0.0), 1e-5));
    REQUIRE(ApproxEq(camera->Generate({ 0.0, 0.0 }).direction, Vec3r(1.0, 0.0, 0.0), 1e-5));
}
