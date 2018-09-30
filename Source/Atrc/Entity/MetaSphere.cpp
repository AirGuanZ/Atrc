#include <Atrc/Entity/MetalSphere.h>
#include <Atrc/Material/BxDF.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

namespace
{
    class MetalBxDF
        : ATRC_IMPLEMENTS BxDF,
          ATRC_PROPERTY AGZ::Uncopiable
    {
        CoordSys localCoord_;
        Spectrum color_;
        Real roughness_;

    public:

        explicit MetalBxDF(const Intersection &inct, const Spectrum &color, Real roughness)
            : localCoord_(CoordSys::FromZ(inct.nor)), color_(color), roughness_(roughness)
        {
            
        }

        BxDFType GetType() const override
        {
            return BXDF_REFLECTION | BXDF_SPECULAR;
        }

        Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override
        {
            return SPECTRUM::BLACK;
        }

        Option<BxDFSample> Sample(const Vec3r &wi, BxDFType type) const override
        {
            if(type.Contains(BXDF_REFLECTION | BXDF_SPECULAR) &&
                Dot(wi, localCoord_.ez) > Real(0))
            {
                Vec3r dir = Real(2) * Dot(localCoord_.ez, wi) * localCoord_.ez - wi;
                if(roughness_)
                    dir = (dir + roughness_ * CommonSampler::Uniform_InUnitSphere::Sample().sample).Normalize();
                Real cos = Dot(dir, localCoord_.ez);
                if(cos <= Real(0))
                    return None;
                return BxDFSample{ dir, color_ / SS(cos), Real(1) };
            }
            return None;
        }
    };
}

MetalSphere::MetalSphere(Real radius, const Transform &local2World, const Spectrum &color, Real roughness)
    : Sphere(radius, local2World), reflectedColor_(color), roughness_(roughness)
{
    
}

RC<BxDF> MetalSphere::GetBxDF(const Intersection &inct) const
{
    return NewRC<MetalBxDF>(inct, reflectedColor_, roughness_);
}

AGZ_NS_END(Atrc)
