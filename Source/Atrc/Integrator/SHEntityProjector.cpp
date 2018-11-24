#include <Atrc/Integrator/SHEntityProjector.h>

AGZ_NS_BEG(Atrc)

SHEntityProjector::SHEntityProjector(int l, int m, int N)
    : sh_(AGZ::Math::GetSHByLM<Real>(l, m)), N_(N)
{
    AGZ_ASSERT(N > 0 && Abs(m) <= l);
}

Spectrum SHEntityProjector::Eval(const Scene &scene, const Ray &r, AGZ::ObjArena<> &arena) const
{
    SurfacePoint sp;
    if(!scene.FindCloestIntersection(r, &sp))
        return Spectrum();

    ShadingPoint shd;
    sp.entity->GetMaterial(sp)->Shade(sp, &shd, arena);

    Spectrum ret;

    for(int i = 0; i < N_; ++i)
    {
        auto bsdfSample = shd.bsdf->SampleWi(sp.wo, BXDF_ALL);
        if(!bsdfSample)
            continue;

        // 过滤掉可见项V为0的采样
        if(scene.HasIntersection(Ray(sp.pos, bsdfSample->wi, EPS)))
            continue;

        auto f    = bsdfSample->coef;
        auto cosV = Dot(sp.geoLocal.ez, bsdfSample->wi);
        auto sh   = sh_(bsdfSample->wi);
        auto pS2  = bsdfSample->pdf;

        ret += f * float(cosV * sh / pS2);
    }

    return ret / N_;
}

AGZ_NS_END(Atrc)
