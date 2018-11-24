#include <Atrc/Integrator/SHLightProjector.h>

AGZ_NS_BEG(Atrc)

SHLightProjector::SHLightProjector(int l, int m, int N)
    : sh_(AGZ::Math::GetSHByLM<Real>(l, m)), N_(N)
{
    AGZ_ASSERT(N > 0 && Abs(m) <= l);
}

Spectrum SHLightProjector::Eval(const Scene &scene, [[maybe_unused]] const Ray &r, [[maybe_unused]] AGZ::ObjArena<> &arena) const
{
    if(scene.GetLights().size() != 1)
        return Spectrum();

    auto light = scene.GetLights().front();
    Spectrum ret;

    for(int i = 0; i < N_; ++i)
    {
        Real u0 = Rand(), u1 = Rand();
        auto[dir, pdf] = AGZ::Math::DistributionTransform::
            UniformOnUnitSphere<Real>::Transform({ u0, u1 });

        auto le = light->NonareaLe(Ray(Vec3(0.0), dir, EPS));
        auto sh = sh_(dir);

        ret += le * float(sh / pdf);
    }

    return ret / N_;
}

AGZ_NS_END(Atrc)
