#include <Atrc/Core/Core/CoordSystem.h>

namespace Atrc
{

CoordSystem CoordSystem::RotateToNewEz(Vec3 newEz) const noexcept
{
    /*newEz = newEz.Normalize();
    if(ApproxEq(ez, newEz, EPS))
        return *this;

    Vec3 rotateAxis = Cross(ez, newEz).Normalize();
    Real theta = Arccos(Clamp(Dot(newEz, ez), Real(-1), Real(1)));
    auto rot = AGZ::Math::Quaternion<Real>::Rotate(rotateAxis, Rad(theta));

    AGZ_ASSERT(ApproxEq(rot.Apply(ez).Normalize(), newEz, Real(1e-3)));

    Vec3 newEx = rot.Apply(ex);
    Vec3 newEy = Cross(newEz, newEx);*/

    auto newEy = Cross(newEz, ex);
    auto newEx = Cross(newEy, newEz);
    return CoordSystem(newEx, newEy, newEz);
}

} // namespace Atrc
