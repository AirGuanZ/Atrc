#include <Atrc/Entity/GeometryTemplate/Sphere.h>

AGZ_NS_BEG(Atrc)

Sphere::Sphere(Real radius)
    : radius_(radius)
{

}

bool Sphere::HasIntersection(const Ray &r) const
{
    return Geometry::Sphere::HasIntersection(r, radius_);
}

bool Sphere::EvalIntersection(const Ray &r, Intersection *inct) const
{
    if(!Geometry::Sphere::EvalIntersection(
        r, radius_, inct))
        return false;
    
    inct->entity = this;
    inct->flag = 0;

    return true;
}

AABB Sphere::GetBoundingBox() const
{
    return { Vec3r(-radius_), Vec3r(radius_) };
}

Real Sphere::SurfaceArea() const
{
    return 4 * PI<Real> * radius_ * radius_;
}

Option<GeometrySurfaceSample> Sphere::SampleSurfaceTo(const Vec3r &dst) const
{
    GeometrySurfaceSample ret;

    for(;;)
    {
        auto [sam, pdf] = CommonSampler::Uniform_InUnitSphere::Sample();
        if(sam.LengthSquare() > 0.05)
        {
            ret.nor = sam.Normalize();
            break;
        }
    }

    ret.pos = radius_ * ret.nor;
    ret.pdf = 1 / (4 * PI<Real>) / (radius_ * radius_);

    return ret;
}

Real Sphere::SampleSurfaceToPDF(const Vec3r &dst, const Vec3r &p) const
{
    return 1 / (4 * PI<Real>) / (radius_ * radius_);
}

AGZ_NS_END(Atrc)
