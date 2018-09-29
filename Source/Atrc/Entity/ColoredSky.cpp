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

        BxDFType GetType() const override
        {
            return BXDF_NONE;
        }

        Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override
        {
            return SPECTRUM::BLACK;
        }

        Option<BxDFSample> Sample(const Vec3r &wi, BxDFType type) const override
        {
            return None;
        }

        [[noreturn]] Real PDF(const Vec3r &wi, const Vec3r &wo) const override
        {
            throw UnreachableCodeException(
                "ColoredSkyBRDF has type BXDF_NONE, "
                "thus ColoredSkyBRDF::PDF shouldn't be called");
        }

        Spectrum AmbientRadiance(const Intersection &inct) const override
        {
            SS t = SS(0.5) * SS(inct.wr.z) + SS(0.5);
            return t * colorTop_ + (SS(1) - t) * colorBottom_;
        }
    };
}

ColoredSky::ColoredSky(const Spectrum &top, const Spectrum &bottom)
    : top_(top), bottom_(bottom)
{

}

bool ColoredSky::HasIntersection(const Ray &r) const
{
    return true;
}

Option<Intersection> ColoredSky::EvalIntersection(const Ray &r) const
{
    AGZ_ASSERT(ApproxEq(r.direction.Length(), Real(1), Real(1e-5)));
    return Intersection {
        -r.direction,
        Vec3r(RealT::Max()),
        -r.direction,
        RealT::Max(),
        this,
    };
}

RC<BxDF> ColoredSky::GetBxDF(const Intersection &inct) const
{
    return NewRC<ColoredSkyBRDF>(top_, bottom_);
}

AGZ_NS_END(Atrc)
