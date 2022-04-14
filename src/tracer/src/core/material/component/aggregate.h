#pragma once

#include "./component.h"

AGZ_TRACER_BEGIN

template<int MAX_COMP_CNT>
class AggregateBSDF : public LocalBSDF
{
    real weights_[MAX_COMP_CNT];

    const BSDFComponent *comps_[MAX_COMP_CNT];

    int comp_cnt_;

    FSpectrum albedo_;

public:

    AggregateBSDF(const FCoord &geometry, const FCoord &shading, const FSpectrum &albedo);

    void add_component(real weight, const BSDFComponent *component);

    FSpectrum eval(const FVec3 &wi, const FVec3 &wo, TransMode mode) const override;

    BSDFSampleResult sample(const FVec3 &wo, TransMode mode, const Sample3 &sam) const override;

    BSDFBidirSampleResult sample_bidir(const FVec3 &wo, TransMode mode, const Sample3 &sam) const override;

    real pdf(const FVec3 &wi, const FVec3 &wo) const override;

    bool is_delta() const override;

    FSpectrum albedo() const override;

    bool has_diffuse_component() const override;
};

template<int MAX_COMP_CNT>
FSpectrum AggregateBSDF<MAX_COMP_CNT>::eval(const FVec3 &wi, const FVec3 &wo, TransMode mode) const
{
    // process black fringes

    if(cause_black_fringes(wi, wo))
        return eval_black_fringes(wi, wo);

    // compute normalized local wi/wo

    const FVec3 lwi = shading_coord_.global_to_local(wi).normalize();
    const FVec3 lwo = shading_coord_.global_to_local(wo).normalize();
    if(!lwi.z || !lwo.z)
        return {};

    // traversal components

    FSpectrum ret;
    for(int i = 0; i < comp_cnt_; ++i)
    {
        const auto comp = comps_[i];
        ret += comp->eval(lwi, lwo, mode);
    }

    return ret;
}

template<int MAX_COMP_CNT>
BSDFSampleResult AggregateBSDF<MAX_COMP_CNT>::sample(const FVec3 &wo, TransMode mode, const Sample3 &sam) const
{
    // process black fringes

    if(cause_black_fringes(wo))
        return sample_black_fringes(wo, mode, sam);

    // compute normalized local wo

    const FVec3 lwo = shading_coord_.global_to_local(wo).normalize();
    if(!lwo.z)
        return BSDF_SAMPLE_RESULT_INVALID;

    // compute sum of weights

    real weight_sum = 0;
    for(int i = 0; i < comp_cnt_; ++i)
    {
        const auto comp = comps_[i];
        weight_sum += weights_[i];
    }
    assert(weight_sum > 0);

    // select a component to sample with

    real comp_selector = (sam.u - real(0.001)) * weight_sum;
    const BSDFComponent *sam_comp = nullptr;
    real weight = 0;

    for(int i = 0; i < comp_cnt_; ++i)
    {
        const auto comp = comps_[i];
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
        if(comp == sam_comp)
            continue;

        sam_ret.f += comp->eval(sam_ret.lwi, lwo, mode);
        sam_ret.pdf += comp->pdf(sam_ret.lwi, lwo) * weights_[i];
    }

    // construct sampling result

    const FVec3 wi = shading_coord_.local_to_global(sam_ret.lwi);
    const real normal_corr_factor = local_angle::normal_corr_factor(
        geometry_coord_, shading_coord_, wi);

    return BSDFSampleResult(wi, sam_ret.f * normal_corr_factor, sam_ret.pdf, false);
}

template<int MAX_COMP_CNT>
BSDFBidirSampleResult AggregateBSDF<MAX_COMP_CNT>::sample_bidir(
    const FVec3 &wo, TransMode mode, const Sample3 &sam) const
{
    // process black fringes

    if(cause_black_fringes(wo))
        return sample_bidir_black_fringes(wo, mode, sam);

    // compute normalized local wo

    const FVec3 lwo = shading_coord_.global_to_local(wo).normalize();
    if(!lwo.z)
        return BSDF_BIDIR_SAMPLE_RESULT_INVALID;

    // compute sum of weights

    real weight_sum = 0;
    for(int i = 0; i < comp_cnt_; ++i)
    {
        const auto comp = comps_[i];
        weight_sum += weights_[i];
    }
    assert(weight_sum > 0);

    // select a component to sample with

    real comp_selector = (sam.u - real(0.001)) * weight_sum;
    const BSDFComponent *sam_comp = nullptr;
    real weight = 0;

    for(int i = 0; i < comp_cnt_; ++i)
    {
        const auto comp = comps_[i];
        if(comp_selector <= weights_[i])
        {
            sam_comp = comp;
            weight = weights_[i];
            break;
        }
        comp_selector -= weights_[i];
    }

    if(!sam_comp)
        return BSDF_BIDIR_SAMPLE_RESULT_INVALID;

    // sample the component

    auto sam_ret = sam_comp->sample_bidir(lwo, mode, { sam.v, sam.w });
    if(!sam_ret.is_valid())
        return BSDF_BIDIR_SAMPLE_RESULT_INVALID;

    // accumulate f/pdf

    sam_ret.lwi = sam_ret.lwi.normalize();
    sam_ret.pdf *= weight;
    sam_ret.pdf_rev *= weight;

    for(int i = 0; i < comp_cnt_; ++i)
    {
        const auto comp = comps_[i];
        if(comp == sam_comp)
            continue;
        sam_ret.f += comp->eval(sam_ret.lwi, lwo, mode);
        sam_ret.pdf += comp->pdf(sam_ret.lwi, lwo) * weights_[i];
        sam_ret.pdf += comp->pdf(lwo, sam_ret.lwi) * weights_[i];
    }

    // construct sampling result

    const FVec3 wi = shading_coord_.local_to_global(sam_ret.lwi);
    const real normal_corr_factor = local_angle::normal_corr_factor(
        geometry_coord_, shading_coord_, wi);

    return BSDFBidirSampleResult(wi, sam_ret.f * normal_corr_factor, sam_ret.pdf, sam_ret.pdf_rev, false);
}

template<int MAX_COMP_CNT>
real AggregateBSDF<MAX_COMP_CNT>::pdf(const FVec3 &wi, const FVec3 &wo) const
{
    // handle black fringes

    if(cause_black_fringes(wi, wo))
        return pdf_for_black_fringes(wi, wo);

    // compute normalized local wi/wo
    
    const FVec3 lwi = shading_coord_.global_to_local(wi).normalize();
    const FVec3 lwo = shading_coord_.global_to_local(wo).normalize();
    if(!lwi.z || !lwo.z)
        return 0;

    // traversal components

    real weight_sum = 0;
    real pdf = 0;

    for(int i = 0; i < comp_cnt_; ++i)
    {
        auto comp = comps_[i];
        weight_sum += weights_[i];
        pdf += weights_[i] * comp->pdf(lwi, lwo);
    }

    return weight_sum > 0 ? pdf / weight_sum : 0;
}

template<int MAX_COMP_CNT>
AggregateBSDF<MAX_COMP_CNT>::AggregateBSDF(const FCoord &geometry, const FCoord &shading, const FSpectrum &albedo)
    : LocalBSDF(geometry, shading)
{
    comp_cnt_ = 0;
    albedo_ = albedo;
}

template<int MAX_COMP_CNT>
void AggregateBSDF<MAX_COMP_CNT>::add_component(real weight, const BSDFComponent *component)
{
    assert(comp_cnt_ < MAX_COMP_CNT);
    weights_[comp_cnt_] = weight;
    comps_[comp_cnt_] = component;
    ++comp_cnt_;
}

template<int MAX_COMP_CNT>
bool AggregateBSDF<MAX_COMP_CNT>::is_delta() const
{
    return false;
}

template<int MAX_COMP_CNT>
FSpectrum AggregateBSDF<MAX_COMP_CNT>::albedo() const
{
    return albedo_;
}

template<int MAX_COMP_CNT>
bool AggregateBSDF<MAX_COMP_CNT>::has_diffuse_component() const
{
    for(int i = 0; i < comp_cnt_; ++i)
    {
        if(comps_[i]->has_diffuse_component())
            return true;
    }
    return false;
}

AGZ_TRACER_END
