#include <iostream>

#include <AGZUtils/Utils/Console.h>
#include <AGZUtils/Utils/Texture.h>
#include <Atrc/Core/Core/Light.h>
#include <Atrc/Mgr/Context.h>
#include <Atrc/Mgr/BuiltinCreatorRegister.h>
#include <Lib/cnpy/cnpy.h>

void Light2SH(const Atrc::Light *light, int SHOrder, Atrc::Spectrum *coefs)
{
    using namespace Atrc;

    AGZ_ASSERT(light && SHOrder >= 0 && SHOrder < 5 && coefs);

    int SHC = (SHOrder + 1) * (SHOrder + 1);
    auto SHTable = AGZ::Math::SH::GetSHTable<Real>();

    for(int i = 0; i < SHC; ++i)
        coefs[i] = Spectrum();

    constexpr int N = 100000;
    int step = (std::max)(10, N / 200);
    AGZ::ProgressBarF pbar(80);

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

        if(n % step == 0 || n == N - 1)
        {
            auto percent = 100 * (static_cast<float>(n + 1) / N);
            pbar.SetPercent(percent);
            pbar.Display();
        }
    }

    for(int i = 0; i < SHC; ++i)
        coefs[i] /= N;

    pbar.Done();
}

void NormalToNpySH(const std::string &imgFilename, int SHOrder, const std::string &outputFilename)
{
    if(SHOrder < 0 || SHOrder > 4)
    {
        std::cout << "Invalid SHOrder value" << std::endl;
        return;
    }

    AGZ::Fmt lightConfig(
        "workspace = \"@.\";"
        "light = {{"
        "   type = Environment;"
        "   tex  = {{"
        "       type = Image;"
        "       filename = \"{}\";"
        "   };"
        "};"
    );
    AGZ::Config config;
    auto configStr = lightConfig.Arg(imgFilename);
    config.LoadFromMemory(configStr);

    Atrc::Mgr::Context ctx(config.Root(), ".");
    RegisterBuiltinCreators(ctx);

    auto light = ctx.Create<Atrc::Light>(config.Root()["light"]);
    int SHC = (SHOrder + 1) * (SHOrder + 1);
    std::vector<Atrc::Spectrum> coefs(SHC);
    Light2SH(light, SHOrder, coefs.data());

    cnpy::npy_save(outputFilename, &coefs[0][0], { size_t(SHC), 3 });
}
