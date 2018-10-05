#include <Atrc/Entity/GeometryTemplate/Sphere.h>

AGZ_NS_BEG(Atrc)

Sphere::Sphere(Real radius, const Transform &local2World)
    : radius_(radius), local2World_(local2World)
{

}

bool Sphere::HasIntersection(const Ray &r) const
{
    return Geometry::Sphere::HasIntersection(
        local2World_.ApplyInverseToRay(r), radius_);
}

bool Sphere::EvalIntersection(const Ray &r, Intersection *inct) const
{
    if(!Geometry::Sphere::EvalIntersection(
        local2World_.ApplyInverseToRay(r), radius_, inct))
        return false;
    
    *inct = local2World_.ApplyToIntersection(*inct);
    inct->entity = this;
    inct->flag = 0;

    return true;
}

AABB Sphere::GetBoundingBox() const
{
    Vec3r r1 = local2World_.ApplyToPoint(Vec3r(-radius_)),
          r2 = local2World_.ApplyToPoint(Vec3r(radius_));
    auto [x1, x2] = std::minmax(r1.x, r2.x);
    auto [y1, y2] = std::minmax(r1.y, r2.y);
    auto [z1, z2] = std::minmax(r1.z, r2.z);
    return { { x1, y1, z1 }, { x2, y2, z2 } };
}

Real Sphere::SurfaceArea() const
{
    return 4 * PI<Real> * radius_ * radius_;
}

AGZ_NS_END(Atrc)
