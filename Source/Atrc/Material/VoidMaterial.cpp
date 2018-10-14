#include <Atrc/Material/BxDF.h>
#include <Atrc/Material/VoidMaterial.h>

AGZ_NS_BEG(Atrc)

namespace
{
    class VoidBxDF
        : ATRC_IMPLEMENTS BxDF
    {
    public:

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

        bool CanScatter() const override
        {
            return false;
        }
    };
}

Box<BxDF> VoidMaterial::GetBxDF(const Intersection &inct, const Vec2r &matParam) const
{
    return NewBox<VoidBxDF>();
}

AGZ_NS_END(Atrc)
