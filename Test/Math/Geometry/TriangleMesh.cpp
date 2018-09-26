#include <Math/Geometry/TriangleAux.h>

#include "../../Catch.hpp"

using namespace Atrc;
using namespace TriangleAux;

TEST_CASE("TriangleAux", "Math")
{
    SECTION("Intersection")
    {
        auto valid = [](const Ray &r, const Option<TriangleIntersection> &inct)
        {
            return !inct || ((r.minT <= inct->t && inct->t <= r.maxT) &&
                (ApproxEq(inct->coefA + inct->coefB + inct->coefC, Real(1.0), 1e-5)));
        };

        {
            Ray r = Ray::New({ -1.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 });
            REQUIRE(valid(r, EvalIntersection2(r, { 1.0, -1.0, -1.0 },
                                                 { 1.0, 1.0, 0.0 },
                                                 { 1.0, -1.0, 1.0 })));
            REQUIRE(!EvalIntersection2(r, { -2.0, -1.0, -1.0 },
                                         { -2.0, 1.0, 0.0 },
                                         { -2.0, -1.0, 1.0 }));
        }
    }
}
