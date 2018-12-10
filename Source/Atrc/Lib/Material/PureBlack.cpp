#include <Atrc/Lib/Material/PureBlack.h>

namespace Atrc
{

namespace
{
    class PureBlackBSDF : public BSDF
    {
    public:

        Spectrum Eval(
            [[maybe_unused]] const CoordSystem &shd, [[maybe_unused]] const CoordSystem &geo,
            [[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &wo) const noexcept
        {
            return Spectrum();
        }

        Option<SampleWiResult> SampleWi(
            [[maybe_unused]] const CoordSystem &shd, [[maybe_unused]] const CoordSystem &geo,
            [[maybe_unused]] const Vec3 &wo) const noexcept
        {
            return None;
        }

        Real SampleWiPDF(
            [[maybe_unused]] const CoordSystem &shd, [[maybe_unused]] const CoordSystem &geo,
            [[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &wo) const noexcept
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
    ret.coordSys = inct.coordSys;
    ret.bsdf     = &BSDF;
    return ret;
}

} // namespace Atrc
