#pragma once

#include <agz/tracer/common.h>
#include <agz-utils/misc.h>

AGZ_TRACER_BEGIN

/**
 * @brief result of sampling BSDF
 */
struct BSDFSampleResult
{
    explicit BSDFSampleResult(uninitialized_t) noexcept { }

    BSDFSampleResult(const FVec3 &dir, const FSpectrum &f, real pdf, bool is_delta) noexcept
        : dir(dir), f(f), pdf(pdf), is_delta(is_delta)
    {

    }

    FVec3         dir;             // scattering direction
    FSpectrum     f;                // bsdf value
    real          pdf = 0;          // pdf w.r.t. solid angle
    bool          is_delta = false; // is f/pdf delta function?

    bool invalid() const noexcept
    {
        return !dir;
    }
};

struct BSDFBidirSampleResult
{
    explicit BSDFBidirSampleResult(uninitialized_t) noexcept { }

    BSDFBidirSampleResult(const FVec3 &dir, const FSpectrum &f, real pdf, real pdf_rev, bool is_delta)
        : dir(dir), f(f), pdf(pdf), pdf_rev(pdf_rev), is_delta(is_delta)
    {
        
    }

    FVec3     dir;
    FSpectrum f;
    real      pdf      = 0;
    real      pdf_rev  = 0;
    bool      is_delta = false;

    bool invalid() const
    {
        return !dir;
    }
};

inline BSDFSampleResult discard_pdf_rev(const BSDFBidirSampleResult &sample_result)
{
    return BSDFSampleResult(sample_result.dir, sample_result.f, sample_result.pdf, sample_result.is_delta);
}

/**
 * @brief returned value when BSDF sampling fails
 */
inline const BSDFSampleResult      BSDF_SAMPLE_RESULT_INVALID       = BSDFSampleResult({}, {}, 0, false);
inline const BSDFBidirSampleResult BSDF_BIDIR_SAMPLE_RESULT_INVALID = BSDFBidirSampleResult({}, {}, 0, 0, false);

/**
 * @brief bidirectional scattering distribution function
 */
class BSDF : public misc::uncopyable_t
{
public:

    virtual ~BSDF() = default;

    /**
     * @brief eval f(in, out) or f*(in, out)
     */
    virtual FSpectrum eval(const FVec3 &wi, const FVec3 &wo, TransMode mode) const = 0;

    /**
     * @brief given wo, sample wi
     */
    virtual BSDFSampleResult sample(const FVec3 &wo, TransMode mode, const Sample3 &sam) const = 0;

    /*
     * @brief same with `sample`, but also returns pdf_rev
     */
    virtual BSDFBidirSampleResult sample_bidir(const FVec3 &wo, TransMode mode, const Sample3 &sam) const = 0;

    /**
     * @brief pdf of sample
     */
    virtual real pdf(const FVec3 &wi, const FVec3 &wo) const = 0;

    /**
     * @brief material albedo
     */
    virtual FSpectrum albedo() const = 0;

    /**
     * @brief is this a delta function?
     */
    virtual bool is_delta() const = 0;

    /**
     * @brief does this bsdf contains a diffuse component?
     */
    virtual bool has_diffuse_component() const = 0;
};

/**
 * @brief BSDF with local frame
 */
class LocalBSDF : public BSDF
{
protected:

    FCoord geometry_coord_;
    FCoord shading_coord_;

    // following 5 methods are helpers for handling black bringes

    bool cause_black_fringes(const FVec3 &w) const noexcept
    {
        const bool shading_posi  = shading_coord_.in_positive_z_hemisphere(w);
        const bool geometry_posi = geometry_coord_.in_positive_z_hemisphere(w);
        return shading_posi != geometry_posi;
    }

    bool cause_black_fringes(const FVec3 &w1, const FVec3 &w2) const
    {
        return cause_black_fringes(w1) || cause_black_fringes(w2);
    }

    FSpectrum eval_black_fringes(const FVec3 &in, const FVec3 &out) const
    {
        if(!geometry_coord_.in_positive_z_hemisphere(in) ||
           !geometry_coord_.in_positive_z_hemisphere(out))
            return FSpectrum();
        return albedo() / PI_r;
    }

    BSDFSampleResult sample_black_fringes(const FVec3 &out, TransMode mode, const Sample3 &sam) const
    {
        if(!geometry_coord_.in_positive_z_hemisphere(out))
            return BSDF_SAMPLE_RESULT_INVALID;

        const auto [lwi, pdf] = math::distribution::
                                    zweighted_on_hemisphere(sam.u, sam.v);
        if(pdf < EPS())
            return BSDF_SAMPLE_RESULT_INVALID;

        const FVec3 wi = geometry_coord_.local_to_global(lwi).normalize();
        const real normal_corr = local_angle::normal_corr_factor(
            geometry_coord_, shading_coord_, wi);
        const FSpectrum f = albedo() / PI_r * normal_corr;

        return BSDFSampleResult(wi, f, pdf, false);
    }

    BSDFBidirSampleResult sample_bidir_black_fringes(const FVec3 &out, TransMode mode, const Sample3 &sam) const
    {
        if(!geometry_coord_.in_positive_z_hemisphere(out))
            return BSDF_BIDIR_SAMPLE_RESULT_INVALID;

        const auto [lwi, pdf] = math::distribution::zweighted_on_hemisphere(sam.u, sam.v);
        if(pdf < EPS())
            return BSDF_BIDIR_SAMPLE_RESULT_INVALID;

        const FVec3 wi = geometry_coord_.local_to_global(lwi).normalize();
        const real normal_corr = local_angle::normal_corr_factor(
            geometry_coord_, shading_coord_, wi);
        const FSpectrum f = albedo() / PI_r * normal_corr;

        const FVec3 lwo = geometry_coord_.global_to_local(out).normalize();
        const real pdf_rev = math::distribution::zweighted_on_hemisphere_pdf(lwo.z);

        return BSDFBidirSampleResult(wi, f, pdf, pdf_rev, false);
    }

    real pdf_for_black_fringes(const FVec3 &in, const FVec3 &out) const
    {
        if(geometry_coord_.in_positive_z_hemisphere(in) !=
           geometry_coord_.in_positive_z_hemisphere(out))
            return false;
        const FVec3 local_in = geometry_coord_.global_to_local(in).normalize();
        return math::distribution::zweighted_on_hemisphere_pdf(local_in.z);
    }

public:

    /**
     * @param geometry_coord geometric local frame, determined by the
     *  mathematically correct geometric properties of the object
     * @param shading_coord shading local frame, constructed by geometry_coord,
     *  normal interpolation and normal mapping
     */
    LocalBSDF(const FCoord &geometry_coord, const FCoord &shading_coord)
        : geometry_coord_(geometry_coord), shading_coord_(shading_coord)
    {

    }
};

AGZ_TRACER_END
