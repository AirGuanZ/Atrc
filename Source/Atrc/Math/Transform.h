#pragma once

#include <Atrc/Common.h>
#include <Atrc/Math/Ray.h>

AGZ_NS_BEG(Atrc)

class Transform
{
    Mat4r mat_, inv_;

public:

    struct FromInv_t { };

    Transform();

    explicit Transform(const Mat4r &mat);

    Transform(const Mat4r &mat, const Mat4r &inv);

    Transform(FromInv_t, const Mat4r &inv);

    Transform operator*(const Transform &rhs) const;

    Transform Inverse() const;

    Vec3r ApplyToPoint(const Vec3r &p) const;
    Vec3r ApplyInverseToPoint(const Vec3r &p) const;

    Vec3r ApplyToVector(const Vec3r &v) const;
    Vec3r ApplyInverseToVector(const Vec3r &v) const;

    Vec3r ApplyToNormal(const Vec3r &n) const;
    Vec3r ApplyInverseToNormal(const Vec3r &n) const;

    Ray ApplyToRay(const Ray &r) const;
    Ray ApplyInverseToRay(const Ray &r) const;

    Intersection ApplyToIntersection(const Intersection &inct) const;
    Intersection ApplyInverseToIntersection(const Intersection &inct) const;
};

inline Transform::FromInv_t FROM_INV;

inline Transform TRANSFORM_IDENTITY(Mat4r::IDENTITY());

inline Transform::Transform()
    : mat_(Real(1)), inv_(Real(1))
{

}

inline Transform::Transform(const Mat4r &mat)
    : mat_(mat), inv_(mat.Inverse())
{

}

inline Transform::Transform(const Mat4r &mat, const Mat4r &inv)
    : mat_(mat), inv_(inv)
{

}

inline Transform::Transform(FromInv_t, const Mat4r &inv)
    : mat_(inv.Inverse()), inv_(inv)
{

}

inline Transform Transform::operator*(const Transform &rhs) const
{
    return Transform(mat_ * rhs.mat_, rhs.inv_ * inv_);
}

inline Transform Transform::Inverse() const
{
    return Transform(inv_, mat_);
}

inline Vec3r Transform::ApplyToPoint(const Vec3r &p) const
{
    return mat_.ApplyToPoint(p);
}

inline Vec3r Transform::ApplyInverseToPoint(const Vec3r &p) const
{
    return inv_.ApplyToPoint(p);
}

inline Vec3r Transform::ApplyToVector(const Vec3r &v) const
{
    return mat_.ApplyToVector(v);
}

inline Vec3r Transform::ApplyInverseToVector(const Vec3r &v) const
{
    return inv_.ApplyToVector(v);
}

inline Vec3r Transform::ApplyToNormal(const Vec3r &n) const
{
    return inv_.ApplyInverseToNormal(n);
}

inline Vec3r Transform::ApplyInverseToNormal(const Vec3r &n) const
{
    return mat_.ApplyInverseToNormal(n);
}

inline Ray Transform::ApplyToRay(const Ray &r) const
{
    return Ray(
        ApplyToPoint(r.origin),
        ApplyToVector(r.direction),
        r.minT, r.maxT);
}

inline Ray Transform::ApplyInverseToRay(const Ray &r) const
{
    return Ray(
        ApplyToPoint(r.origin),
        ApplyToVector(r.direction),
        r.minT, r.maxT);
}

inline Intersection Transform::ApplyToIntersection(const Intersection &inct) const
{
    return Intersection{
        ApplyToVector(inct.wr),
        ApplyToPoint(inct.pos),
        ApplyToNormal(inct.nor),
        inct.t, inct.entity, inct.flag
    };
}

inline Intersection Transform::ApplyInverseToIntersection(const Intersection &inct) const
{
    return Intersection{
        ApplyInverseToVector(inct.wr),
        ApplyInverseToPoint(inct.pos),
        ApplyInverseToNormal(inct.nor),
        inct.t, inct.entity, inct.flag
    };
}

AGZ_NS_END(Atrc)
