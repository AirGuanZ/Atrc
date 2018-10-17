#include <Atrc/Common.h>
#include <Atrc/Entity/ColoredSky.h>
#include <Atrc/Material/BxDF.h>

AGZ_NS_BEG(Atrc)

namespace
{
    class ColoredSkyBRDF
        : ATRC_IMPLEMENTS BxDF,
          ATRC_PROPERTY AGZ::Uncopiable
    {
        Spectrum colorTop_;
        Spectrum colorBottom_;

    public:

        ColoredSkyBRDF(const Spectrum &top, const Spectrum &bottom)
            : colorTop_(top), colorBottom_(bottom)
        {

        }

        Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override
        {
            return SPECTRUM::BLACK;
        }

        Real PDF(const Vec3r &wi, const Vec3r &wo) const override
        {
            return 0.0;
        }

        Option<BxDFSample> Sample(const Vec3r &wi) const override
        {
            return None;
        }

        Spectrum AmbientRadiance(const Intersection &inct) const override
        {
            SS t = SS(0.5) * SS(inct.wr.z) + SS(0.5);
            return (SS(1) - t) * colorTop_ + t * colorBottom_;
        }

        bool CanScatter() const override
        {
            return false;
        }
    };
}

ColoredSky::ColoredSky(const Spectrum &topAndBottom)
    : ColoredSky(topAndBottom, topAndBottom)
{

}

ColoredSky::ColoredSky(const Spectrum &top, const Spectrum &bottom)
    : top_(top), bottom_(bottom)
{

}

bool ColoredSky::HasIntersection(const Ray &r) const
{
    return RealT(r.maxT).IsInfinity();
}

bool ColoredSky::EvalIntersection(const Ray &r, Intersection *inct) const
{
    AGZ_ASSERT(ApproxEq(r.direction.Length(), 1.0, 1e-5));

    inct->wr     = -r.direction;
    inct->pos    = Vec3r(RealT::Max());
    inct->nor    = -r.direction;
    inct->t      = RealT::Max();
    inct->entity = this;

    return true;
}

AABB ColoredSky::GetBoundingBox() const
{
    return AABB();
}

Real ColoredSky::SurfaceArea() const
{
    return RealT::Infinity();
}

RC<BxDF> ColoredSky::GetBxDF(const Intersection &inct) const
{
    return NewRC<ColoredSkyBRDF>(top_, bottom_);
}

AGZ_NS_END(Atrc)
