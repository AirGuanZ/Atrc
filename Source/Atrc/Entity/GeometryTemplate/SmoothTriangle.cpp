#include <Atrc/Entity/GeometryTemplate/SmoothTriangle.h>

AGZ_NS_BEG(Atrc)

SmoothTriangle::SmoothTriangle(const Vec3r (&vertices)[3], const Vec3r (&normals)[3])
    : A(vertices[0]), B_A(vertices[1] - vertices[0]), C_A(vertices[2] - vertices[0]),
      nA(normals[0]), nB_nA(normals[1] - normals[0]), nC_nA(normals[2] - normals[0])
{

}

bool SmoothTriangle::HasIntersection(const Ray &r) const
{
    return Geometry::Triangle::HasIntersection(A, B_A, C_A, r);
}

bool SmoothTriangle::EvalIntersection(const Ray &r, Intersection *inct) const
{
    AGZ_ASSERT(inct);
    if(!Geometry::Triangle::EvalIntersection(A, B_A, C_A, r, inct))
        return false;
    inct->nor = (nA + inct->uv.u * nB_nA + inct->uv.v * nC_nA).Normalize();
    inct->entity = this;
    return true;
}

AABB SmoothTriangle::GetBoundingBox() const
{
    Vec3r B = A + B_A, C = A + C_A;
    return {
        {
            Min(A.x, Min(B.x, C.x)),
            Min(A.y, Min(B.y, C.y)),
            Min(A.z, Min(B.z, C.z))
        },
        {
            Max(A.x, Max(B.x, C.x)),
            Max(A.y, Max(B.y, C.y)),
            Max(A.z, Max(B.z, C.z))
        }
    };
}

Real SmoothTriangle::SurfaceArea() const
{
    return Geometry::Triangle::SurfaceArea(B_A, C_A);
}

AGZ_NS_END(Atrc)
