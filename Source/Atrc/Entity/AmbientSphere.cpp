#include <Atrc/Entity/AmbientSphere.h>
#include <Atrc/Material/BxDF.h>

AGZ_NS_BEG(Atrc)

namespace
{
    class AmbientBRDF
        : ATRC_IMPLEMENTS BxDF,
          ATRC_PROPERTY AGZ::Uncopiable
    {
        Spectrum color_;

    public:

        explicit AmbientBRDF(const Spectrum &color)
            : color_(color)
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

        Spectrum AmbientRadiance(const Intersection &inct) const override
        {
            return color_;
        }
    };
}

AmbientSphere::AmbientSphere(Real radius, const Transform &local2World, const Spectrum &color)
    : Sphere(radius, local2World), ambientColor_(color)
{

}

RC<BxDF> AmbientSphere::GetBxDF(const Intersection &inct) const
{
    return NewRC<AmbientBRDF>(ambientColor_);
}

AGZ_NS_END(Atrc)
