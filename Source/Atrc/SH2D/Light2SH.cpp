#include <Atrc/Lib/Core/Light.h>
#include <Atrc/Lib/Core/Ray.h>
#include <Atrc/SH2D/Light2SH.h>

using namespace Atrc;

void Light2SH(const Light *light, int SHOrder, int N, Spectrum *coefs)
{
    AGZ_ASSERT(light && SHOrder >= 0 && SHOrder < 5 && N > 0 && coefs);
    
    int SHC = (SHOrder + 1) * (SHOrder + 1);
    auto SHTable = AGZ::Math::SH::GetSHTable<Real>();

    for(int i = 0; i < SHC; ++i)
        coefs[i] = Spectrum();

    int step = (std::max)(10, N / 50);

    for(int n = 0; n < N; ++n)
    {
        Real u = AGZ::Math::Random::Uniform<Real>(0, 1);
        Real v = AGZ::Math::Random::Uniform<Real>(0, 1);
        auto [dir, pdf] = AGZ::Math::DistributionTransform::
            UniformOnUnitSphere<Real>::Transform({ u, v });
        if(pdf <= 0)
            continue;

        for(int i = 0; i < SHC; ++i)
            coefs[i] += light->NonAreaLe(Ray(Vec3(), dir, EPS)) * SHTable[i](dir) / pdf;

        if(n % step == 0)
            std::cout << "Progress: " << 100 * (static_cast<double>(n + 1) / N) << "%" << std::endl;
    }

    for(int i = 0; i < SHC; ++i)
        coefs[i] /= N;
}
