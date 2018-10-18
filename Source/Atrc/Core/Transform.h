#pragma once

#include <Atrc/Core/Common.h>

#include <Atrc/Core/AABB.h>
#include <Atrc/Core/Ray.h>

AGZ_NS_BEG(Atrc)

class Transform
{
    Mat4 mat_;
    Mat4 inv_;

public:

    explicit Transform(const Mat4 &mat)
        : mat_(mat), inv_(mat.Inverse())
    {
        
    }

    Transform(const Mat4 &mat, const Mat4 &inv)
        : mat_(mat), inv_(inv)
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

    static Transform Scale(const Vec3 &scale)
    {
        AGZ_ASSERT(scale.x > 0.0 && scale.y > 0.0 && scale.z > 0.0);
        return Transform(Mat4::Scale(scale),
                         Mat4::Scale(scale.Map([](Real v) { return 1.0 / v; })));
    }

    static Transform Scale(Real x, Real y, Real z)
    {
        return Scale({ x, y, z });
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
        return inv_.ApplyInverseToNormal(nor);
    }

    Vec3 ApplyInverseToNormal(const Vec3 &nor) const
    {
        return mat_.ApplyInverseToNormal(nor);
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
};

AGZ_NS_END(Atrc)
