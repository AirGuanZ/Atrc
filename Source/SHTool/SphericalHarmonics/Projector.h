#include "SphericalHarmonics.h"

template<int L, int M>
class EntityProjector
{
public:

    // 若r未击中任何一点，返回None
    // 否则，设交点为x，返回以x和-r.dir为参数的t系数
    static Option<Spectrum> Project(const Ray &r, const Scene &scene);
};

template<int L, int M>
class EnvironmentLightProjector
{
public:

    static Spectrum Project(const Light *envLight);
};

template<int L, int M>
Option<Spectrum> EntityProjector<L, M>::Project(const Ray &r, const Scene &scene)
{
    using namespace Atrc;

    SurfacePoint inct;
    if(!scene.FindCloestIntersection(r, &inct))
        return None;

    // TODO
    return None;
}
