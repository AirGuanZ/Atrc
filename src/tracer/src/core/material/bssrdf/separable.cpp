#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/utility/reflection.h>

#include "./separable.h"

AGZ_TRACER_BEGIN

namespace
{

    real fresnel_moment(real eta)
    {
        const real eta2 = eta * eta;
        const real eta3 = eta2 * eta;
        const real eta4 = eta2 * eta2;
        const real eta5 = eta2 * eta3;

        if(eta < 1)
        {
            return real(0.45966)
                 - real(1.73965)  * eta
                 + real(3.37668)  * eta2
                 - real(3.904945) * eta3
                 + real(2.49277)  * eta4
                 - real(0.68441)  * eta5;
        }

        return real(-4.61686)
             + real(11.1136) * eta
             - real(10.4646) * eta2
             + real(5.11455) * eta3
             - real(1.27198) * eta4
             + real(0.12746) * eta5;
    }

    class SeparableBSDF : public BSDF
    {
        FCoord coord_;
        real eta_;

    public:

        SeparableBSDF(const FCoord &coord, real eta)
            : coord_(coord), eta_(eta)
        {
            
        }

        FSpectrum eval(
            const FVec3 &wi, const FVec3&, TransMode, uint8_t) const noexcept override
        {
            const real cosThetaI = cos(wi, coord_.z);

            const real cI = 1 - 2 * fresnel_moment(eta_);

            const real fr = refl_aux::dielectric_fresnel(eta_, 1, cosThetaI);
            const real val = (1 - fr) / (cI * PI_r);

            return FSpectrum(val);
        }

        BSDFSampleResult sample(
            const FVec3&, TransMode, const Sample3 &sam, uint8_t) const noexcept override
        {
            const auto [local_dir, pdf] = math::distribution::
                    zweighted_on_hemisphere(sam.u, sam.v);

            const FVec3 dir = coord_.local_to_global(local_dir);

            return BSDFSampleResult(
                dir,
                eval_all(dir, {}, TransMode::Radiance),
                pdf,
                false);
        }

        real pdf(const FVec3 &wi, const FVec3&, uint8_t) const noexcept override
        {
            const FVec3 lwi = coord_.global_to_local(wi).normalize();
            return math::distribution::zweighted_on_hemisphere_pdf(lwi.z);
        }

        FSpectrum albedo() const noexcept override
        {
            return {};
        }

        bool is_delta() const noexcept override
        {
            return false;
        }

        bool has_diffuse_component() const noexcept override
        {
            return true;
        }
    };

    class SeparableBSDFMaterial : public Material
    {
        const BSDF *bsdf_;

    public:

        explicit SeparableBSDFMaterial(const BSDF *bsdf)
            : bsdf_(bsdf)
        {
            
        }

        ShadingPoint shade(
            const EntityIntersection &inct, Arena &arena) const override
        {
            ShadingPoint shd;
            shd.shading_normal = inct.geometry_coord.z;
            shd.bsdf           = bsdf_;
            shd.bssrdf         = nullptr;
            return shd;
        }
    };

} // namespace anonymous

SeparableBSSRDF::SeparableBSSRDF(const EntityIntersection &po, real eta)
    : eta_(eta), po_(po)
{
    
}

BSSRDFSamplePiResult SeparableBSSRDF::sample_pi(
    const Sample3 &sam, Arena &arena) const
{
    // color channel

    auto [channel, sample_u] = math::distribution::extract_uniform_int(
        sam.u, 0, SPECTRUM_COMPONENT_COUNT);
    
    // construct proj coord

    FCoord proj_coord;

    real sample_v;
    if(sam.v < 0.5)
    {
        proj_coord = po_.geometry_coord;
        sample_v = 2 * sam.v;
    }
    else if(sam.v < 0.75)
    {
        proj_coord = FCoord(
            po_.geometry_coord.y, po_.geometry_coord.z, po_.geometry_coord.x);
        sample_v = 4 * (sam.v - real(0.5));
    }
    else
    {
        proj_coord = FCoord(
            po_.geometry_coord.z, po_.geometry_coord.x, po_.geometry_coord.y);
        sample_v = 4 * (sam.v - real(0.75));
    }

    // sample r and phi

    const auto sr = sample_r(channel, { sample_u });
    const real r_max = sample_r(channel, { real(0.996) }).distance;
    if(sr.distance <= 0 || sr.distance > r_max)
        return BSSRDF_SAMPLE_PI_RESULT_INVALID;
    const real phi = 2 * PI_r * sample_v;

    // construct inct ray

    const real hl = std::sqrt(std::max(
        real(0), r_max * r_max - sr.distance * sr.distance));
    FVec3 inct_ray_ori(sr.distance * std::cos(phi), sr.distance * std::sin(phi), hl);
    real inct_ray_len = 2 * hl;

    Ray inct_ray(
        po_.pos + proj_coord.local_to_global(inct_ray_ori),
        -proj_coord.z,
        EPS(), std::max(EPS(), inct_ray_len));

    // find all incts

    struct InctListNode
    {
        EntityIntersection inct;
        InctListNode *next = nullptr;

    } *inct_list_entry = nullptr;

    int inct_cnt = 0;

    for(;;)
    {
        if(inct_ray.t_min >= inct_ray.t_max)
            break;

        EntityIntersection new_inct;
        if(!po_.entity->closest_intersection(inct_ray, &new_inct))
            break;

        if(new_inct.material == po_.material)
        {
            static_assert(std::is_trivially_destructible_v<InctListNode>);
            auto nNode = arena.create<InctListNode>();
            nNode->inct = new_inct;
            nNode->next = inct_list_entry;
            inct_list_entry = nNode;
            ++inct_cnt;
        }

        inct_ray.o = inct_ray.at(new_inct.t + EPS());
        inct_ray.t_max -= new_inct.t + EPS();
    }

    if(!inct_cnt)
        return BSSRDF_SAMPLE_PI_RESULT_INVALID;

    // select an inct

    auto [inct_idx, sample_w] = math::distribution::extract_uniform_int(
        sam.w, 0, inct_cnt);
    for(int i = 0; i < inct_idx; ++i)
    {
        assert(inct_list_entry);
        inct_list_entry = inct_list_entry->next;
        assert(inct_list_entry);
    }

    if(inct_list_entry->inct.material != po_.material)
        return BSSRDF_SAMPLE_PI_RESULT_INVALID;

    // construct ret

    const real pdf_radius = pdf_pi(inct_list_entry->inct);

    const BSDF *bsdf = arena.create_nodestruct<SeparableBSDF>(
        inct_list_entry->inct.geometry_coord, eta_);

    const real cos_theta_o = cos(po_.wr, po_.geometry_coord.z);
    const real fro = 1 - refl_aux::dielectric_fresnel(eta_, 1, cos_theta_o);

    EntityIntersection inct = inct_list_entry->inct;
    inct.material = arena.create_nodestruct<SeparableBSDFMaterial>(bsdf);

    const FSpectrum coef = fro * eval_r(distance(inct.pos, po_.pos));
    const real pdf = pdf_radius / (2 * PI_r) / inct_cnt;

    return BSSRDFSamplePiResult(inct, coef, pdf);
}

real SeparableBSSRDF::pdf_pi(const EntityIntersection &pi) const
{
    const FVec3 ld = po_.geometry_coord.global_to_local(po_.pos - pi.pos);
    const FVec3 ln = po_.geometry_coord.global_to_local(pi.geometry_coord.z);
    const real r_proj[] = {
        Vec2(ld.y, ld.z).length(),
        Vec2(ld.z, ld.x).length(),
        Vec2(ld.x, ld.y).length()
    };

    constexpr real AXIS_PDF[] = { real(0.25), real(0.25), real(0.5) };
    constexpr real CHANNEL_PDF = real(1) / SPECTRUM_COMPONENT_COUNT;

    real ret = 0;
    for(int axis_idx = 0; axis_idx < 3; ++axis_idx)
    {
        for(int ch_idx = 0; ch_idx < SPECTRUM_COMPONENT_COUNT; ++ch_idx)
        {
            ret += std::abs(ln[axis_idx]) *
                   pdf_r(ch_idx, r_proj[axis_idx]) * AXIS_PDF[axis_idx];
        }
    }

    return ret * CHANNEL_PDF;
}

AGZ_TRACER_END
