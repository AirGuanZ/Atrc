#include <Math/Geometry/Sphere.h>

#include "../../Catch.hpp"

using namespace Atrc;

TEST_CASE("Sphere", "Math")
{
    SECTION("HasIntersection")
    {
        REQUIRE(Sphere(2.0).HasIntersection(Ray::New(
                                Vec3r(-5.0, 0.0, 0.0),
                                Vec3r::UNIT_X())));
    }

    SECTION("EvalIntersection")
    {
        REQUIRE(ApproxEq(Sphere(20)
                            .EvalIntersection(Ray::New(
                                Vec3r(-50, 0.0, 0.0),
                                Vec3r(1.0, 0.0, 0.0)))
                            .value().local.normal,
                         -Vec3r::UNIT_X(),
                         1e-5));
    }
}
