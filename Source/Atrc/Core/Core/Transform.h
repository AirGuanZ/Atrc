#pragma once

#include <Atrc/Core/Core/AABB.h>
#include <Atrc/Core/Core/SurfacePoint.h>

namespace Atrc
{

class Transform
{
    Mat4 mat_, inv_;
    Real scaleFac_, invScaleFac_;

    Transform(const Mat4 &mat, const Mat4 &inv) noexcept;

public:

    static Transform Translate(const Vec3 &offset) noexcept;

    static Transform Rotate(const Vec3 &axis, Rad angle) noexcept;

    static Transform RotateX(Rad angle) noexcept;

    static Transform RotateY(Rad angle) noexcept;

    static Transform RotateZ(Rad angle) noexcept;

    static Transform Scale(Real scale) noexcept;

    Transform();

    const Mat4 &GetMatrix() const noexcept { return mat_; }

    Real ScaleFactor() const noexcept;

    Real InverseScaleFactor() const noexcept;

    Transform operator*(const Transform &rhs) const noexcept;

    Vec3 ApplyToPoint(const Vec3 &p) const noexcept;

    Vec3 ApplyInverseToPoint(const Vec3 &p) const noexcept;

    Vec3 ApplyToVector(const Vec3 &v) const noexcept;

    Vec3 ApplyInverseToVector(const Vec3 &v) const noexcept;

    Ray ApplyToRay(const Ray &r) const noexcept;

    Ray ApplyInverseToRay(const Ray &r) const noexcept;

    AABB ApplyToAABB(const AABB &aabb) const noexcept;

    AABB ApplyInverseToAABB(const AABB &aabb) const noexcept;

    CoordSystem ApplyToCoordSystem(const CoordSystem &c) const noexcept;

    CoordSystem ApplyInverseToCoordSystem(const CoordSystem &c) const noexcept;
};

// ================================= Implementation

inline Transform::Transform(const Mat4 &mat, const Mat4 &inv) noexcept
    : mat_(mat), inv_(inv), scaleFac_(mat.ApplyToVector(Vec3::UNIT_X()).Length())
{
    invScaleFac_ = 1 / scaleFac_;
}

inline Transform Transform::Translate(const Vec3 &offset) noexcept
{
    return Transform(Mat4::Translate(offset), Mat4::Translate(-offset));
}

inline Transform Transform::Rotate(const Vec3 &axis, Rad angle) noexcept
{
    return Transform(Mat4::Rotate(axis, angle), Mat4::Rotate(axis, -angle));
}

inline Transform Transform::RotateX(Rad angle) noexcept
{
    return Transform(Mat4::RotateX(angle), Mat4::RotateX(-angle));
}

inline Transform Transform::RotateY(Rad angle) noexcept
{
    return Transform(Mat4::RotateY(angle), Mat4::RotateY(-angle));
}

inline Transform Transform::RotateZ(Rad angle) noexcept
{
    return Transform(Mat4::RotateZ(angle), Mat4::RotateZ(-angle));
}

inline Transform Transform::Scale(Real scale) noexcept
{
    return Transform(Mat4::Scale(Vec3(scale)), Mat4::Scale(Vec3(1 / scale)));
}

inline Transform::Transform()
    : mat_(Mat4::IDENTITY()), inv_(Mat4::IDENTITY()), scaleFac_(1), invScaleFac_(1)
{

}

inline Real Transform::ScaleFactor() const noexcept
{
    return scaleFac_;
}

inline Real Transform::InverseScaleFactor() const noexcept
{
    return invScaleFac_;
}

inline Transform Transform::operator*(const Transform &rhs) const noexcept
{
    return Transform(mat_ * rhs.mat_, rhs.inv_ * inv_);
}

inline Vec3 Transform::ApplyToPoint(const Vec3 &p) const noexcept
{
    return mat_.ApplyToPoint(p);
}

inline Vec3 Transform::ApplyInverseToPoint(const Vec3 &p) const noexcept
{
    return inv_.ApplyToPoint(p);
}

inline Vec3 Transform::ApplyToVector(const Vec3 &v) const noexcept
{
    return mat_.ApplyToVector(v);
}

inline Vec3 Transform::ApplyInverseToVector(const Vec3 &v) const noexcept
{
    return inv_.ApplyToVector(v);
}

inline Ray Transform::ApplyToRay(const Ray &r) const noexcept
{
    Ray ret = r;
    ret.o = ApplyToPoint(r.o);
    ret.d = ApplyToVector(r.d);
    return ret;
}

inline Ray Transform::ApplyInverseToRay(const Ray &r) const noexcept
{
    Ray ret = r;
    ret.o = ApplyInverseToPoint(r.o);
    ret.d = ApplyInverseToVector(r.d);
    return ret;
}

inline AABB Transform::ApplyToAABB(const AABB &aabb) const noexcept
{
    AABB ret;
    ret.Expand(ApplyToPoint(aabb.low))
       .Expand(ApplyToPoint(Vec3(aabb.high.x, aabb.low.y,  aabb.low.z)))
       .Expand(ApplyToPoint(Vec3(aabb.low.x,  aabb.high.y, aabb.low.z)))
       .Expand(ApplyToPoint(Vec3(aabb.low.x,  aabb.low.y,  aabb.high.z)))
       .Expand(ApplyToPoint(Vec3(aabb.high.x, aabb.high.y, aabb.low.z)))
       .Expand(ApplyToPoint(Vec3(aabb.low.x,  aabb.high.y, aabb.high.z)))
       .Expand(ApplyToPoint(Vec3(aabb.high.x, aabb.low.y,  aabb.high.z)))
       .Expand(ApplyToPoint(aabb.high));
    return ret;
}

inline AABB Transform::ApplyInverseToAABB(const AABB &aabb) const noexcept
{
    AABB ret;
    ret.Expand(ApplyInverseToPoint(aabb.low))
       .Expand(ApplyInverseToPoint(Vec3(aabb.high.x, aabb.low.y,  aabb.low.z)))
       .Expand(ApplyInverseToPoint(Vec3(aabb.low.x,  aabb.high.y, aabb.low.z)))
       .Expand(ApplyInverseToPoint(Vec3(aabb.low.x,  aabb.low.y,  aabb.high.z)))
       .Expand(ApplyInverseToPoint(Vec3(aabb.high.x, aabb.high.y, aabb.low.z)))
       .Expand(ApplyInverseToPoint(Vec3(aabb.low.x,  aabb.high.y, aabb.high.z)))
       .Expand(ApplyInverseToPoint(Vec3(aabb.high.x, aabb.low.y,  aabb.high.z)))
       .Expand(ApplyInverseToPoint(aabb.high));
    return ret;
}

inline CoordSystem Transform::ApplyToCoordSystem(const CoordSystem &c) const noexcept
{
    return CoordSystem(ApplyToVector(c.ex), ApplyToVector(c.ey), ApplyToVector(c.ez));
}

inline CoordSystem Transform::ApplyInverseToCoordSystem(const CoordSystem &c) const noexcept
{
    return CoordSystem(ApplyToVector(c.ex), ApplyToVector(c.ey), ApplyToVector(c.ez));
}

} // namespace Atrc
