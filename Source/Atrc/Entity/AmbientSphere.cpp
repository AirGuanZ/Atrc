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

AmbientSphere::AmbientSphere(Real radius, const Spectrum &color, const Transform &local2World)
    : radius_(radius), ambientColor_(color), local2World_(local2World)
{

}

bool AmbientSphere::HasIntersection(const Ray &r) const
{
    return Geometry::Sphere::HasIntersection(
        local2World_.ApplyInverseToRay(r), radius_);
}

bool AmbientSphere::EvalIntersection(const Ray &r, Intersection *inct) const
{
    if(!Geometry::Sphere::EvalIntersection(
        local2World_.ApplyInverseToRay(r), radius_, inct))
        return false;

    *inct = local2World_.ApplyToIntersection(*inct);
    inct->entity = this;
    inct->flag = 0;

    return true;
}

RC<BxDF> AmbientSphere::GetBxDF(const Intersection &inct) const
{
    return NewRC<AmbientBRDF>(ambientColor_);
}

AGZ_NS_END(Atrc)
