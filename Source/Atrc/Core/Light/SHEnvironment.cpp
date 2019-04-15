#include <AGZUtils/Utils/Math.h>
#include <Atrc/Core/Light/SHEnvironment.h>

namespace Atrc
{
    
SHEnvLight::SHEnvLight(int SHOrder, const Spectrum *coefs, Real rotateDeg)
    : InfiniteLight(Transform::RotateY(Deg(rotateDeg))), SHOrder_(SHOrder)
{
    int SHC = SHOrder * SHOrder;
    SHCoefs_.resize(SHC);
    for(int i = 0; i < SHC; ++i)
        SHCoefs_[i] = coefs[i];
}

Spectrum SHEnvLight::NonAreaLe(const Ray &r) const noexcept
{
    static const auto SHTable = AGZ::Math::SH::GetSHTable<Real>();
    Vec3 d = local2World_.ApplyInverseToVector(r.d).Normalize();
    Spectrum ret;
    for(size_t i = 0; i < SHCoefs_.size(); ++i)
        ret += SHCoefs_[i] * SHTable[i](d);
    return ret;
}

} // namespace Atrc
