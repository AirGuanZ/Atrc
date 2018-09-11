#include <iostream>

#define AGZ_ALL_IMPL
#include <Utils.h>

#include "Math/Geometry.h"
#include "Math/Geometry/Sphere.h"

using namespace std;
using namespace Atrc;

int main()
{
    Transform trans;
    [[maybe_unused]]
    TransformWrapper t(trans, new Sphere(&trans, 1.0));

    cout << "Hello, world!" << endl;
}
