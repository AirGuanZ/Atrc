#include <Math/Geometry/Sphere.h>

#include "../../Catch.hpp"

using namespace Atrc;

TEST_CASE("Sphere", "Math")
{
    SECTION("HasIntersection")
    {
        REQUIRE(Sphere(2.0).HasIntersection(Ray(
                                Vec3r(-5.0, 0.0, 0.0),
                                Vec3r::UNIT_X())));
    }

    SECTION("EvalIntersection")
    {
        REQUIRE(ApproxEq(Sphere(20)
                            .EvalIntersection(Ray(
                                Vec3r(-50, 0.0, 0.0),
                                Vec3r(1.0, 0.0, 0.0)))
                            .value().inct.normal,
                         -Vec3r::UNIT_X(),
                         1e-5));

        Transform rot(Mat4r::RotateZ(Deg<Real>(45.567))
                    * Mat4r::RotateX(Deg<Real>(23.546)));
        REQUIRE(ApproxEq(Sphere(20, &rot)
                            .EvalIntersection(Ray(
                                Vec3r(-50, 0.0, 0.0),
                                Vec3r(1.0, 0.0, 0.0)))
                            .value().inct.normal,
                         -Vec3r::UNIT_X(),
                         1e-5));


        Transform rot2(Mat4r::RotateZ(Deg<Real>(45.57))
                     * Mat4r::RotateX(Deg<Real>(23.546))
                     * Mat4r::Rotate(Vec3r(1.0, 2.0, 3.0).Normalize(), Deg<Real>(124.45789)));
        REQUIRE(ApproxEq(Sphere(20, &rot2)
                            .EvalIntersection(Ray(
                                Vec3r(-50, 0.0, 0.0),
                                Vec3r(1.0, 0.0, 0.0)))
                            .value().inct.normal,
                         -Vec3r::UNIT_X(),
                         1e-5));
    }
}
