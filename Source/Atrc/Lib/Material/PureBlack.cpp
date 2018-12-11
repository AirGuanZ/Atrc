#include <Atrc/Lib/Core/Entity.h>
#include <Atrc/Lib/Material/PureBlack.h>

namespace Atrc
{

namespace
{
    class PureBlackBSDF : public BSDF
    {
    public:

        Spectrum GetAlbedo([[maybe_unused]] BSDFType type) const noexcept override
        {
            return Spectrum();
        }

        Spectrum Eval(
            [[maybe_unused]] const CoordSystem &shd, [[maybe_unused]] const CoordSystem &geo,
            [[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &wo) const noexcept override
        {
            return Spectrum();
        }

        Option<SampleWiResult> SampleWi(
            [[maybe_unused]] const CoordSystem &shd, [[maybe_unused]] const CoordSystem &geo,
            [[maybe_unused]] const Vec3 &wo) const noexcept override
        {
            return None;
        }

        Real SampleWiPDF(
            [[maybe_unused]] const CoordSystem &shd, [[maybe_unused]] const CoordSystem &geo,
            [[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &wo) const noexcept override
        {
            return 0;
        }
    };
}

ShadingPoint PureBlack::GetShadingPoint(const Intersection &inct, [[maybe_unused]] Arena &arena) const noexcept
{
    static const PureBlackBSDF BSDF;
    ShadingPoint ret;
    ret.uv       = inct.uv;
    ret.coordSys = inct.entity->GetShadingCoordSys(inct);
    ret.bsdf     = &BSDF;
    return ret;
}

} // namespace Atrc
