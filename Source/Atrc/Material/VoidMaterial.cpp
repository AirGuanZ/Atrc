#include <Atrc/Material/BxDF.h>
#include <Atrc/Material/VoidMaterial.h>

AGZ_NS_BEG(Atrc)

namespace
{
    class VoidBxDF
        : ATRC_IMPLEMENTS BxDF
    {
    public:

        Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override
        {
            return SPECTRUM::BLACK;
        }

        Option<BxDFSample> Sample(const Vec3r &wi) const override
        {
            return None;
        }

        bool CanScatter() const override
        {
            return false;
        }
    };
}

RC<BxDF> VoidMaterial::GetBxDF(const Intersection &inct, const Vec2r &matParam) const
{
    return NewRC<VoidBxDF>();
}

AGZ_NS_END(Atrc)
