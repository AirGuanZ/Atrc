#include <Atrc/Core/Geometry.h>

AGZ_NS_BEG(Atrc)

bool Geometry::HasIntersection(const Ray &r) const
{
    SurfacePoint sp;
    return FindIntersection(r, &sp);
}

AABB Geometry::WorldBound() const
{
    return local2World_.ApplyToAABB(LocalBound());
}

GeometrySampleResult Geometry::Sample([[maybe_unused]] const Vec3 &dst) const
{
    return Sample();
}

Real Geometry::SamplePDF(const Vec3 &pos, [[maybe_unused]] const Vec3 &dst) const
{
    return SamplePDF(pos);
}

AGZ_NS_END(Atrc)
