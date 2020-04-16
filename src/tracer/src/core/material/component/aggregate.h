#pragma once

#include "./component.h"

AGZ_TRACER_BEGIN

template<int MAX_COMP_CNT>
class AggregateBSDF : public LocalBSDF
{
    real weights_[MAX_COMP_CNT];

    const BSDFComponent *comps_[MAX_COMP_CNT];

    int comp_cnt_;

    Spectrum albedo_;

public:

    AggregateBSDF(
        const Coord &geometry, const Coord &shading,
        const Spectrum &albedo) noexcept;

    void add_component(real weight, const BSDFComponent *component) noexcept;

    Spectrum eval(
        const Vec3 &wi, const Vec3 &wo,
        TransMode mode, uint8_t type) const noexcept;

    BSDFSampleResult sample(
        const Vec3 &wo, TransMode mode,
        const Sample3 &sam, uint8_t type) const noexcept;

    real pdf(
        const Vec3 &wi, const Vec3 &wo,
        uint8_t type) const noexcept;

    Spectrum eval_all(
        const Vec3 &wi, const Vec3 &wo, TransMode mode) const noexcept override;

    BSDFSampleResult sample_all(
        const Vec3 &wo, TransMode mode,
        const Sample3 &sam) const noexcept override;

    real pdf_all(const Vec3 &wi, const Vec3 &wo) const noexcept override;

    bool is_delta() const noexcept override;

    Spectrum albedo() const noexcept override;

    bool has_diffuse_component() const noexcept override;
};

template<int MAX_COMP_CNT>
Spectrum AggregateBSDF<MAX_COMP_CNT>::eval(
    const Vec3 &wi, const Vec3 &wo,
    TransMode mode, uint8_t type) const noexcept
{
    // process black fringes

    if(cause_black_fringes(wi, wo))
        return eval_black_fringes(wi, wo);

    // compute normalized local wi/wo

    const Vec3 lwi = shading_coord_.global_to_local(wi).normalize();
    const Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
    if(!lwi.z || !lwo.z)
        return {};

    // traversal components

    Spectrum ret;
    for(int i = 0; i < comp_cnt_; ++i)
    {
        const auto comp = comps_[i];
        if(comp->is_contained_in(type))
            ret += comp->eval(lwi, lwo, mode);
    }

    return ret;
}

template<int MAX_COMP_CNT>
BSDFSampleResult AggregateBSDF<MAX_COMP_CNT>::sample(
    const Vec3 &wo, TransMode mode, const Sample3 &sam,
    uint8_t type) const noexcept
{
    // process black fringes

    if(cause_black_fringes(wo))
        return sample_black_fringes(wo, mode, sam);

    // compute normalized local wo

    const Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
    if(!lwo.z)
        return {};

    // compute sum of weights

    real weight_sum = 0;
    for(int i = 0; i < comp_cnt_; ++i)
    {
        const auto comp = comps_[i];
        if(comp->is_contained_in(type))
            weight_sum += weights_[i];
    }

    if(!weight_sum)
        return BSDF_SAMPLE_RESULT_INVALID;

    // select a component to sample with

    real comp_selector = (sam.u - real(0.001)) * weight_sum;
    const BSDFComponent *sam_comp = nullptr;
    real weight = 0;

    for(int i = 0; i < comp_cnt_; ++i)
    {
        const auto comp = comps_[i];
        if(!comp->is_contained_in(type))
            continue;

        if(comp_selector <= weights_[i])
        {
            sam_comp = comp;
            weight = weights_[i];
            break;
        }
        comp_selector -= weights_[i];
    }

    if(!sam_comp)
        return BSDF_SAMPLE_RESULT_INVALID;

    // sample the component

    auto sam_ret = sam_comp->sample(lwo, mode, { sam.v, sam.w });
    if(!sam_ret.is_valid())
        return BSDF_SAMPLE_RESULT_INVALID;

    // accumulate f/pdf

    sam_ret.lwi = sam_ret.lwi.normalize();
    sam_ret.pdf *= weight;

    for(int i = 0; i < comp_cnt_; ++i)
    {
        const auto comp = comps_[i];
        if(comp == sam_comp || !comp->is_contained_in(type))
            continue;

        sam_ret.f += comp->eval(sam_ret.lwi, lwo, mode);
        sam_ret.pdf += comp->pdf(sam_ret.lwi, lwo) * weights_[i];
    }

    // construct sampling result

    const Vec3 wi = shading_coord_.local_to_global(sam_ret.lwi);
    const real normal_corr_factor = local_angle::normal_corr_factor(
        geometry_coord_, shading_coord_, wi);

    BSDFSampleResult ret;
    ret.dir      = wi;
    ret.f        = sam_ret.f * normal_corr_factor;
    ret.pdf      = sam_ret.pdf;
    ret.is_delta = false;

    if(ret.pdf < real(0.001))
        return BSDF_SAMPLE_RESULT_INVALID;

    return ret;
}

template<int MAX_COMP_CNT>
real AggregateBSDF<MAX_COMP_CNT>::pdf(
    const Vec3 &wi, const Vec3 &wo, uint8_t type) const noexcept
{
    // handle black fringes

    if(cause_black_fringes(wi, wo))
        return pdf_for_black_fringes(wi, wo);

    // compute normalized local wi/wo
    
    const Vec3 lwi = shading_coord_.global_to_local(wi).normalize();
    const Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
    if(!lwi.z || !lwo.z)
        return 0;

    // traversal components

    real weight_sum = 0;
    real pdf = 0;

    for(int i = 0; i < comp_cnt_; ++i)
    {
        auto comp = comps_[i];
        if(!comp->is_contained_in(type))
            continue;

        weight_sum += weights_[i];
        pdf += weights_[i] * comp->pdf(lwi, lwo);
    }

    return weight_sum > 0 ? pdf / weight_sum : 0;
}

template<int MAX_COMP_CNT>
AggregateBSDF<MAX_COMP_CNT>::AggregateBSDF(
    const Coord &geometry, const Coord &shading,
    const Spectrum &albedo) noexcept
    : LocalBSDF(geometry, shading)
{
    comp_cnt_ = 0;
    albedo_ = albedo;
}

template<int MAX_COMP_CNT>
void AggregateBSDF<MAX_COMP_CNT>::add_component(
    real weight, const BSDFComponent *component) noexcept
{
    assert(comp_cnt_ < MAX_COMP_CNT);
    weights_[comp_cnt_] = weight;
    comps_[comp_cnt_] = component;
    ++comp_cnt_;
}

template<int MAX_COMP_CNT>
Spectrum AggregateBSDF<MAX_COMP_CNT>::eval_all(
    const Vec3 &wi, const Vec3 &wo, TransMode mode) const noexcept
{
    // process black fringes

    if(cause_black_fringes(wi, wo))
        return eval_black_fringes(wi, wo);

    // compute normalized local wi/wo

    const Vec3 lwi = shading_coord_.global_to_local(wi).normalize();
    const Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
    if(!lwi.z || !lwo.z)
        return {};

    // traversal components

    Spectrum ret;
    for(int i = 0; i < comp_cnt_; ++i)
        ret += comps_[i]->eval(lwi, lwo, mode);

    return ret;
}

template<int MAX_COMP_CNT>
BSDFSampleResult AggregateBSDF<MAX_COMP_CNT>::sample_all(
    const Vec3 &wo, TransMode mode, const Sample3 &sam) const noexcept
{
    // process black fringes

    if(cause_black_fringes(wo))
        return sample_black_fringes(wo, mode, sam);

    // compute normalized local wo

    const Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
    if(!lwo.z)
        return {};

    // compute sum of weights

    real weight_sum = 0;
    for(int i = 0; i < comp_cnt_; ++i)
        weight_sum += weights_[i];

    if(!weight_sum)
        return BSDF_SAMPLE_RESULT_INVALID;

    // select a component to sample with

    real comp_selector = (sam.u - real(0.001)) * weight_sum;
    const BSDFComponent *sam_comp = nullptr;
    real weight = 0;

    for(int i = 0; i < comp_cnt_; ++i)
    {
        if(comp_selector <= weights_[i])
        {
            sam_comp = comps_[i];
            weight = weights_[i];
            break;
        }
        comp_selector -= weights_[i];
    }

    if(!sam_comp)
        return BSDF_SAMPLE_RESULT_INVALID;

    // sample the component

    auto sam_ret = sam_comp->sample(lwo, mode, { sam.v, sam.w });
    if(!sam_ret.is_valid())
        return BSDF_SAMPLE_RESULT_INVALID;

    // accumulate f/pdf

    sam_ret.lwi = sam_ret.lwi.normalize();
    sam_ret.pdf *= weight;

    for(int i = 0; i < comp_cnt_; ++i)
    {
        if(comps_[i] != sam_comp)
        {
            sam_ret.f += comps_[i]->eval(sam_ret.lwi, lwo, mode);
            sam_ret.pdf += comps_[i]->pdf(sam_ret.lwi, lwo) * weights_[i];
        }
    }

    // construct sampling result

    const Vec3 wi = shading_coord_.local_to_global(sam_ret.lwi);
    const real normal_corr_factor = local_angle::normal_corr_factor(
        geometry_coord_, shading_coord_, wi);

    BSDFSampleResult ret;
    ret.dir      = wi;
    ret.f        = sam_ret.f * normal_corr_factor;
    ret.pdf      = sam_ret.pdf;
    ret.is_delta = false;

    if(ret.pdf < real(0.001))
        return BSDF_SAMPLE_RESULT_INVALID;

    return ret;

}

template<int MAX_COMP_CNT>
real AggregateBSDF<MAX_COMP_CNT>::pdf_all(
    const Vec3 &wi, const Vec3 &wo) const noexcept
{
    // handle black fringes

    if(cause_black_fringes(wi, wo))
        return pdf_for_black_fringes(wi, wo);

    // compute normalized local wi/wo
    
    const Vec3 lwi = shading_coord_.global_to_local(wi).normalize();
    const Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
    if(!lwi.z || !lwo.z)
        return 0;

    // traversal components

    real weight_sum = 0;
    real pdf = 0;

    for(int i = 0; i < comp_cnt_; ++i)
    {
        weight_sum += weights_[i];
        pdf += weights_[i] * comps_[i]->pdf(lwi, lwo);
    }

    return weight_sum > 0 ? pdf / weight_sum : 0;
}

template<int MAX_COMP_CNT>
bool AggregateBSDF<MAX_COMP_CNT>::is_delta() const noexcept
{
    return false;
}

template<int MAX_COMP_CNT>
Spectrum AggregateBSDF<MAX_COMP_CNT>::albedo() const noexcept
{
    return albedo_;
}

template<int MAX_COMP_CNT>
bool AggregateBSDF<MAX_COMP_CNT>::has_diffuse_component() const noexcept
{
    for(int i = 0; i < comp_cnt_; ++i)
    {
        if(comps_[i]->get_component_type() & BSDF_DIFFUSE)
            return true;
    }
    return false;
}

AGZ_TRACER_END
