#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/utility/reflection.h>
#include "./separable.h"

AGZ_TRACER_BEGIN

namespace separable_bssrdf_impl
{
    
    real fresnel_moment(real eta) noexcept
    {
        real eta2 = eta * eta;
        real eta3 = eta2 * eta;
        real eta4 = eta3 * eta;
        real eta5 = eta4 * eta;
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

} // namespace separable_bssrdf_impl

class SeparableBSDF : public LocalBSDF
{
    const SeparableBSSRDF *bssrdf_;

public:

    SeparableBSDF(const Coord &geometry_coord, const Coord &shading_coord, const SeparableBSSRDF *bssrdf)
        : LocalBSDF(geometry_coord, shading_coord), bssrdf_(bssrdf)
    {
        assert(bssrdf);
    }

    Spectrum eval(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept override
    {
        if(shading_coord_.in_positive_z_hemisphere(wo) || !shading_coord_.in_positive_z_hemisphere(wi))
            return {};

        real cos_theta_i = cos(wi, shading_coord_.z);
        real fi = bssrdf_->sw(-cos_theta_i);
        if(mode == TM_Radiance)
            fi *= math::sqr(bssrdf_->eta_);
        return Spectrum(fi) * local_angle::normal_corr_factor(geometry_coord_, shading_coord_, wi);
    }

    BSDFSampleResult sample(const Vec3 &wo, TransportMode mode, const Sample3 &sam) const noexcept override
    {
        if(cause_black_fringes(wo))
            return sample_for_black_fringes(wo, mode, sam);

        if(shading_coord_.in_positive_z_hemisphere(wo))
            return BSDF_SAMPLE_RESULT_INVALID;

        auto [local_in, pdf] = math::distribution::zweighted_on_hemisphere(sam.u, sam.v);
        if(pdf < EPS)
            return BSDF_SAMPLE_RESULT_INVALID;

        BSDFSampleResult ret;
        ret.dir      = shading_coord_.local_to_global(local_in).normalize();
        ret.f        = eval(ret.dir, wo, mode);
        ret.pdf      = pdf;
        ret.mode     = mode;
        ret.is_delta = false;

        if(!ret.f.is_finite() || ret.pdf < EPS)
            return BSDF_SAMPLE_RESULT_INVALID;
        
        return ret;
    }

    real pdf(const Vec3 &in_dir, const Vec3 &out_dir, TransportMode) const noexcept override
    {
        if(cause_black_fringes(in_dir, out_dir))
            return pdf_for_black_fringes(in_dir, out_dir);
        if(shading_coord_.in_positive_z_hemisphere(out_dir) || !shading_coord_.in_positive_z_hemisphere(in_dir))
            return 0;
        Vec3 local_in = shading_coord_.global_to_local(in_dir).normalize();
        return math::distribution::zweighted_on_hemisphere_pdf(local_in.z);
    }

    Spectrum albedo() const noexcept override
    {
        return {};
    }

    bool is_black() const noexcept override
    {
        return false;
    }
};

real SeparableBSSRDF::sw(real cos_theta_i) const noexcept
{
    real fi_nor_factor = 1 / ((1 - 2 * separable_bssrdf_impl::fresnel_moment(1 / eta_)) * PI_r);
    real fi = 1 - refl_aux::dielectric_fresnel(eta_, 1, std::abs(cos_theta_i));
    return fi * fi_nor_factor;
}

SeparableBSSRDF::SeparableBSSRDF(const EntityIntersection &inct, const Coord &geometry_coord, const Coord &shading_coord, real eta, real transparency) noexcept
    : BSSRDF(inct), geometry_coord_(geometry_coord), shading_coord_(shading_coord), eta_(eta), transparency_(transparency)
{

}

BSSRDFSampleResult SeparableBSSRDF::sample(const Vec3 &xo_wi, TransportMode mode, const Sample4 &sam, Arena &arena) const
{
    if(transparency_ == 1 || sam.u < transparency_)
    {
        Ray r(xo_.pos, xo_wi.normalize(), EPS);

        BSSRDFSampleResult ret;
        if(!xo_.entity->closest_intersection(r, &ret.inct))
            return BSSRDF_SAMPLE_RESULT_INVALID;
        ret.bsdf = ret.inct.material->shade(ret.inct, arena).bsdf;
        ret.f    = Spectrum(transparency_);
        ret.pdf  = transparency_;

        return ret;
    }


    // choose sampling axis

    int sample_axis;
    real select_axis_u = (1 - sam.u) / (1 - transparency_);
    if(select_axis_u < real(0.5))
        sample_axis = 2;
    else if(select_axis_u < real(0.75))
        sample_axis = 0;
    else
        sample_axis = 1;

    Coord sample_coord;
    if(sample_axis == 0)
        sample_coord = Coord(geometry_coord_.y, geometry_coord_.z, geometry_coord_.x);
    else if(sample_axis == 1)
        sample_coord = Coord(geometry_coord_.z, geometry_coord_.x, geometry_coord_.y);
    else
        sample_coord = Coord(geometry_coord_.x, geometry_coord_.y, geometry_coord_.z);

    // choose sampling channel

    auto [channel, new_v] = math::distribution::extract_uniform_int(sam.v, 0, SPECTRUM_COMPONENT_COUNT);

    // sample distance and phi

    real r = sample_distance(channel, { new_v });
    real r_end = sample_distance(channel, { real(0.997) });
    if(r < 0 || r >= r_end)
        return BSSRDF_SAMPLE_RESULT_INVALID;
    real phi = 2 * PI_r * sam.w;

    // find potential xi

    real l = 2 * std::sqrt(r_end * r_end - r * r);
    Vec3 ray_o = xo_.pos + r * (sample_coord.x * std::cos(phi) + sample_coord.y * std::sin(phi)) - real(0.5) * l * sample_coord.z;
    Ray ray(ray_o, sample_coord.z, 0, l - EPS);

    struct IntersectionListNode
    {
        EntityIntersection inct;
        IntersectionListNode *next = nullptr;
    };
    int list_len = 0;
    IntersectionListNode *list = arena.create<IntersectionListNode>();
    IntersectionListNode *cur_node = list;

    for(;;)
    {
        if(ray.t_min + EPS > ray.t_max ||
           !xo_.entity->closest_intersection(ray, &cur_node->inct) ||
           cur_node->inct.material != xo_.material)
        {
            break;
        }

        ray.t_min = cur_node->inct.t + EPS;

        ++list_len;
        auto *new_node = arena.create<IntersectionListNode>();
        cur_node->next = new_node;
        cur_node = new_node;
    }

    // choose our xi

    if(!list_len)
        return BSSRDF_SAMPLE_RESULT_INVALID;

    BSSRDFSampleResult ret;

    int xi_idx = math::distribution::extract_uniform_int(sam.r, 0, list_len).first;
    while(xi_idx-- > 0)
        list = list->next;
    ret.inct    = list->inct;
    ret.inct.wr = -ret.inct.geometry_coord.z;
    ret.f       = (1 - transparency_) * distance_factor(r);
    ret.pdf     = pdf(ret.inct, mode) / list_len * (1 - transparency_);
    ret.bsdf    = arena.create<SeparableBSDF>(ret.inct.geometry_coord, ret.inct.geometry_coord, this);

    if(!ret.f.is_finite() || ret.pdf < EPS)
        return BSSRDF_SAMPLE_RESULT_INVALID;

    return ret;
}

real SeparableBSSRDF::pdf(const SurfacePoint &xi, TransportMode mode) const noexcept
{
    Vec3 d  = xo_.pos - xi.pos;
    Vec3 ld = geometry_coord_.global_to_local(d);
    Vec3 ln = geometry_coord_.global_to_local(xi.geometry_coord.z);

    real axis_prob[3] = { real(0.25), real(0.25), real(0.5) };
    real channel_prob = real(1) / SPECTRUM_COMPONENT_COUNT;
    real r_proj[3] = { ld.yz().length(), ld.xz().length(), ld.xy().length() };
    
    real ret = 0;
    for(int axis = 0; axis < 3; ++axis)
    {
        for(int channel = 0; channel < SPECTRUM_COMPONENT_COUNT; ++channel)
        {
            real r = (std::max)(r_proj[axis], EPS);
            ret += pdf_distance(channel, r)
                 * std::abs(ln[axis]) * channel_prob * axis_prob[axis] / (2 * PI_r * r);
        }
    }

    return ret;
}

AGZ_TRACER_END
