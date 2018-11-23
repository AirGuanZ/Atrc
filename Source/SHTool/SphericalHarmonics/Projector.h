#pragma once

#include "SphericalHarmonics.h"

template<int L, int M>
class EntityProjector
{
    AGZ::ObjArena<> matArena_;

public:

    // 求以sp.pos和sp.wo为参数的t系数值
    Spectrum Project(const SurfacePoint &sp, const Scene &scene, int N);
};

template<int L, int M>
class EnvironmentLightProjector
{
public:

    static Spectrum Project(const Light *envLight, int N);
};

template<int L, int M>
Spectrum EntityProjector<L, M>::Project(const SurfacePoint &sp, const Scene &scene, int N)
{
    using namespace Atrc;

    ShadingPoint shd;
    sp.entity->GetMaterial(sp)->Shade(sp, &shd, matArena_);

    Spectrum ret;
    
    for(int i = 0; i < N; ++i)
    {
        auto bsdfSample = shd.bsdf->SampleWi(sp.wo, BXDF_ALL);
        if(!bsdfSample)
            continue;

        // 过滤掉可见项V为0的采样
        if(scene.HasIntersection(Ray(sp.pos, bsdfSample->wi, EPS)))
            continue;

        auto f    = bsdfSample->coef;
        auto cosV = Dot(sp.geoLocal.ez, bsdfSample->wi);
        auto sh   = SH<L, M>(bsdfSample->wi);
        auto pS2  = bsdfSample->pdf;

        ret += f * cosV * sh / pS2;
    }

    if(matArena_.GetUsedBytes() >= 1024 * 1024 * 32)
        matArena_.Clear();

    return ret / N;
}

template<int L, int M>
Spectrum EnvironmentLightProjector<L, M>::Project(const Light *envLight, int N)
{
    using namespace Atrc;

    Spectrum ret;

    for(int i = 0; i < N; ++i)
    {
        Real u0 = Rand(), u1 = Rand();
        auto [dir, pdf] = AGZ::Math::DistributionTransform::
                            UniformOnUnitSphere<Real>::Transform({ u0, u1 });
        
        auto le = envLight->NonareaLe(Ray(Vec3(0.0), dir, EPS));
        auto sh = SH<L, M>(dir);

        ret += le * sh / pdf;
    }

    return ret / N;
}
