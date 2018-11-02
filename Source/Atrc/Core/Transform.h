#pragma once

#include <Atrc/Core/Common.h>

#include <Atrc/Core/AABB.h>
#include <Atrc/Core/Ray.h>
#include <Atrc/Core/SurfacePoint.h>

AGZ_NS_BEG(Atrc)

class Transform
{
    Mat4 mat_;
    Mat4 inv_;

    Real scaleFactor_;
    Real invScaleFac_;

    explicit Transform(const Mat4 &mat)
        : mat_(mat), inv_(mat.Inverse()),
          scaleFactor_(mat.ApplyToVector(Vec3::UNIT_X()).LengthSquare())
    {
        invScaleFac_ = 1 / scaleFactor_;
    }

    Transform(const Mat4 &mat, const Mat4 &inv)
        : mat_(mat), inv_(inv),
          scaleFactor_(mat.ApplyToVector(Vec3::UNIT_X()).LengthSquare())
    {
        invScaleFac_ = 1 / scaleFactor_;
    }

public:

    Transform()
        : mat_(Mat4::IDENTITY()), inv_(Mat4::IDENTITY()),
          scaleFactor_(1), invScaleFac_(1)
    {

    }

    static Transform Translate(const Vec3 &offset)
    {
        return Transform(Mat4::Translate(offset), Mat4::Translate(-offset));
    }

    static Transform Translate(Real x, Real y, Real z)
    {
        return Translate({ x, y, z });
    }

    static Transform Rotate(const Vec3 &axis, Rad angle)
    {
        return Transform(Mat4::Rotate(axis, angle), Mat4::Rotate(axis, -angle));
    }

    static Transform RotateX(Rad angle)
    {
        return Transform(Mat4::RotateX(angle), Mat4::RotateX((-angle)));
    }

    static Transform RotateY(Rad angle)
    {
        return Transform(Mat4::RotateY(angle), Mat4::RotateY(-angle));
    }

    static Transform RotateZ(Rad angle)
    {
        return Transform(Mat4::RotateZ(angle), Mat4::RotateZ(-angle));
    }

    static Transform Scale(Real factor)
    {
        Real invFac = 1 / factor;
        return Transform(Mat4::Scale({ factor, factor, factor }),
                         Mat4::Scale({ invFac, invFac, invFac }));
    }

    Real ScaleFactor() const { return scaleFactor_; }
    Real InverseScaleFactor() const { return invScaleFac_; }

    Transform operator*(const Transform &rhs) const
    {
        return Transform(mat_ * rhs.mat_, rhs.inv_ * inv_);
    }

    Vec3 ApplyToPoint(const Vec3 &pnt) const
    {
        return mat_.ApplyToPoint(pnt);
    }

    Vec3 ApplyInverseToPoint(const Vec3 &pnt) const
    {
        return inv_.ApplyToPoint(pnt);
    }

    Vec3 ApplyToVector(const Vec3 &vec) const
    {
        return mat_.ApplyToVector(vec);
    }

    Vec3 ApplyInverseToVector(const Vec3 &vec) const
    {
        return inv_.ApplyToVector(vec);
    }

    Vec3 ApplyToNormal(const Vec3 &nor) const
    {
        return inv_.ApplyInverseToNormal(nor).Normalize();
    }

    Vec3 ApplyInverseToNormal(const Vec3 &nor) const
    {
        return mat_.ApplyInverseToNormal(nor).Normalize();
    }

    Ray ApplyToRay(const Ray &r) const
    {
        return Ray(
            ApplyToPoint(r.ori),
            ApplyToVector(r.dir),
            r.minT, r.maxT);
    }

    Ray ApplyInverseToRay(const Ray &r) const
    {
        return Ray(
            ApplyInverseToPoint(r.ori),
            ApplyInverseToVector(r.dir),
            r.minT, r.maxT);
    }

    AABB ApplyToAABB(const AABB &aabb) const
    {
        AABB ret;
        if(aabb.IsEmpty())
            return ret;
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

    AABB ApplyInverseToAABB(const AABB &aabb) const
    {
        AABB ret;
        if(aabb.IsEmpty())
            return ret;
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

    LocalCoordSystem ApplyToCoordSystem(const LocalCoordSystem &sys) const
    {
        return {
            ApplyToVector(sys.ex).Normalize(),
            ApplyToVector(sys.ey).Normalize(),
            ApplyToVector(sys.ez).Normalize()
        };
    }

    LocalCoordSystem ApplyInverseToCoordSystem(const LocalCoordSystem &sys) const
    {
        return {
            ApplyInverseToVector(sys.ex).Normalize(),
            ApplyInverseToVector(sys.ey).Normalize(),
            ApplyInverseToVector(sys.ez).Normalize()
        };
    }
};

AGZ_NS_END(Atrc)
