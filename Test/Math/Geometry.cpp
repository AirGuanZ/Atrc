#include <Math/Geometry/Sphere.h>

#include "../Catch.hpp"

using namespace Atrc;

TEST_CASE("Transform", "Math")
{
    REQUIRE(ApproxEq(Vec3r(2.0, 4.0, 8.0),
                     Transform(Mat4r::Translate({ 1.0, 2.0, 5.0 }))
                         .ApplyToPoint({ 1.0, 2.0, 3.0 }),
                     1e-5));

    REQUIRE(ApproxEq(Vec3r(0.0, 1.0, 0.0),
                     Transform(Mat4r::RotateZ(PIr / 2))
                         .ApplyToVector({ 1.0,  0.0,  0.0 }),
                     1e-5));

    REQUIRE(ApproxEq(Vec3r(1.0, 0.0, 0.0),
                     Transform(Mat4r::RotateZ(PIr / 2))
                         .Inverse()
                         .ApplyToVector({ 0.0,  1.0,  0.0 }),
                     1e-5));

    REQUIRE(ApproxEq(Transform(Mat4r::RotateZ(PIr / 2))
                        .ApplyToVector({ 2.0, -1.0, 0.0 })
                        .Normalize(),
                     Transform(Mat4r::Scale({ 1.0, 0.5, 1.0 }))
                        .ApplyToNormal(Vec3r(1.0, 1.0, 0.0).Normalize()),
                     1e-5));
}
