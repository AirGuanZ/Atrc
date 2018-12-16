#pragma once

#include <Atrc/Lib/Core/Common.h>

namespace Atrc
{

class CoordSystem
{
public:

    Vec3 ex, ey, ez;

    CoordSystem() = default;

    CoordSystem(const Vec3 &ex, const Vec3 &ey, const Vec3 &ez) noexcept;

    static CoordSystem FromEz(const Vec3 &ez) noexcept;

    Vec3 Local2World(const Vec3 &local) const noexcept;

    Vec3 World2Local(const Vec3 &world) const noexcept;

    CoordSystem World2Local(const CoordSystem &world) const noexcept;

    bool InPositiveHemisphere(const Vec3 &v) const noexcept;

    CoordSystem RotateToNewEz(Vec3 newEz) const noexcept;
};

// ================================= Implementation

inline CoordSystem::CoordSystem(const Vec3 &ex, const Vec3 &ey, const Vec3 &ez) noexcept
    : ex(ex.Normalize()), ey(ey.Normalize()), ez(ez.Normalize())
{

}

inline CoordSystem CoordSystem::FromEz(const Vec3 &ez) noexcept
{
    Vec3 ex;
    if(ApproxEq(Abs(Dot(ez, Vec3::UNIT_X())), Real(1), Real(0.1)))
        ex = Cross(ez, Vec3::UNIT_Y()).Normalize();
    else
        ex = Cross(ez, Vec3::UNIT_X()).Normalize();
    return CoordSystem(ex, Cross(ez, ex).Normalize(), ez.Normalize());
}

inline Vec3 CoordSystem::Local2World(const Vec3 &local) const noexcept
{
    return local.x * ex + local.y * ey + local.z * ez;
}

inline Vec3 CoordSystem::World2Local(const Vec3 &world) const noexcept
{
    return Vec3(Dot(ex, world), Dot(ey, world), Dot(ez, world));
}

inline CoordSystem CoordSystem::World2Local(const CoordSystem &world) const noexcept
{
    return CoordSystem(World2Local(world.ex), World2Local(world.ey), World2Local(world.ez));
}

inline bool CoordSystem::InPositiveHemisphere(const Vec3 &v) const noexcept
{
    return Dot(ez, v) > 0;
}

inline CoordSystem CoordSystem::RotateToNewEz(Vec3 newEz) const noexcept
{
    newEz = newEz.Normalize();
    if(ApproxEq(ez, newEz, EPS))
        return *this;

    Vec3 rotateAxis = Cross(ez, newEz);
    Real theta = Arccos(Dot(newEz, ez));
    auto rot = AGZ::Math::Quaternion<Real>::Rotate(rotateAxis, Rad(theta));
    AGZ_ASSERT(ApproxEq(rot.Apply(ez), newEz, Real(1e-3)));

    Vec3 newEx = rot.Apply(ex);
    Vec3 newEy = Cross(newEz, newEx);

    return CoordSystem(newEx, newEy, newEz);
}

} // namespace Atrc
