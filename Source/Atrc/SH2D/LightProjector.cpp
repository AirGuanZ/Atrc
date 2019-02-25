#include <Atrc/Lib/Core/Light.h>
#include <Atrc/Lib/Core/Ray.h>
#include <Atrc/SH2D/LightProjector.h>

namespace Atrc::SH2D
{

LightProjector::LightProjector(int SHOrder) noexcept
    : SHC_(SHOrder * SHOrder)
{
    AGZ_ASSERT(0 < SHOrder && SHOrder <= 5);
}

void LightProjector::Project(const Light *light, int sampleCount, Spectrum *coefs) const
{
    AGZ_ASSERT(light && coefs && sampleCount > 0);

    auto SHTable = AGZ::Math::SH::GetSHTable<Real>();

    for(int i = 0; i < SHC_; ++i)
        coefs[i] = Spectrum();

    for(int n = 0; n < sampleCount; ++n)
    {
        Real u = AGZ::Math::Random::Uniform<Real>(0, 1);
        Real v = AGZ::Math::Random::Uniform<Real>(0, 1);
        auto [dir, pdf] = AGZ::Math::DistributionTransform
            ::UniformOnUnitSphere<Real>::Transform({ u, v });
        
        for(int i = 0; i < SHC_; ++i)
            coefs[i] += light->NonAreaLe(Ray(Vec3(), dir, EPS)) * SHTable[i](dir) / pdf;
    }

    if(sampleCount > 0)
    {
        for(int i = 0; i < SHC_; ++i)
            coefs[i] /= sampleCount;
    }
}

} // namespace Atrc::SH2D
