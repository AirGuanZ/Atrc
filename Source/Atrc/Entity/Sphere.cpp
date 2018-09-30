#include <Atrc/Entity/Sphere.h>

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

AGZ_NS_END(Atrc)
