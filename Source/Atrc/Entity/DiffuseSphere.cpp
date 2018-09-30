#include <Atrc/Entity/DiffuseSphere.h>
#include <Atrc/Material/BxDF.h>
#include <Atrc/Math/Math.h>
#include "AmbientSphere.h"

AGZ_NS_BEG(Atrc)

namespace
{
    class DiffuseBxDF
        : ATRC_IMPLEMENTS BxDF,
          ATRC_PROPERTY AGZ::Uncopiable
    {
        CoordSys localCoord_;
        Spectrum color_;

    public:

        explicit DiffuseBxDF(const Intersection &inct, const Spectrum &color)
            : localCoord_(CoordSys::FromZ(inct.nor)), color_(color)
        {
            
        }

        BxDFType GetType() const override
        {
            return BXDF_REFLECTION | BXDF_DIFFUSE;
        }

        Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override
        {
            if(Dot(wi, localCoord_.ez) > Real(0) && Dot(wo, localCoord_.ez) > Real(0))
                return color_;
            return SPECTRUM::BLACK;
        }

        Option<BxDFSample> Sample(const Vec3r &wi, BxDFType type) const override
        {
            if(type.Contains(BXDF_REFLECTION | BXDF_DIFFUSE) &&
               Dot(wi, localCoord_.ez) > Real(0))
            {
                auto [dir, pdf] = CommonSampler::ZWeighted_OnUnitHemisphere::Sample();
                return BxDFSample{ localCoord_.C2W(dir), color_, pdf };
            }
            return None;
        }
    };
}

DiffuseSphere::DiffuseSphere(Real radius, const Spectrum &color, const Transform &local2World)
    : radius_(radius), diffuseColor_(color), local2World_(local2World)
{

}

bool DiffuseSphere::HasIntersection(const Ray &r) const
{
    return Geometry::Sphere::HasIntersection(
        local2World_.ApplyInverseToRay(r), radius_);
}

bool DiffuseSphere::EvalIntersection(const Ray &r, Intersection *inct) const
{
    if(!Geometry::Sphere::EvalIntersection(
        local2World_.ApplyInverseToRay(r), radius_, inct))
        return false;

    *inct = local2World_.ApplyToIntersection(*inct);
    inct->entity = this;
    inct->flag = 0;

    return true;
}

RC<BxDF> DiffuseSphere::GetBxDF(const Intersection &inct) const
{
    return NewRC<DiffuseBxDF>(inct, diffuseColor_);
}

AGZ_NS_END(Atrc)
