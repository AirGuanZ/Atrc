#include <Atrc/Lib/Core/Medium.h>

namespace Atrc
{

class HomogeneousMedium : public Medium
{
    Spectrum sigmaT_;
    Spectrum sigmaS_;
    Spectrum le_;
    Real g_;

public:

    HomogeneousMedium(const Spectrum &sigmaA, const Spectrum &sigmaS, const Spectrum &le, Real g) noexcept;

    Spectrum Tr(const Vec3 &a, const Vec3 &b) const override;

    Spectrum TrToInf(const Vec3 &a, const Vec3 &d) const override;

    SampleLsResult SampleLs(const Ray &r, const Vec3 &sample) const override;

    MediumShadingPoint GetShadingPoint(const MediumPoint &medPnt, Arena &arena) const override;
};

} // namespace Atrc
