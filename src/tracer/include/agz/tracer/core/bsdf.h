#pragma once

#include <agz/tracer/common.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

enum BSDFComponentType : uint8_t
{
    BSDF_DIFFUSE    = 1 << 0,
    BSDF_GLOSSY     = 1 << 1,
    BSDF_SPECULAR   = 1 << 2
};

constexpr uint8_t BSDF_ALL = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR;

/**
 * @brief result of sampling BSDF
 */
struct BSDFSampleResult
{
    explicit BSDFSampleResult(uninitialized_t) noexcept { }

    BSDFSampleResult(
        const FVec3 &dir, const FSpectrum &f,
        real pdf, bool is_delta) noexcept
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

/**
 * @brief returned value when BSDF sampling fails
 */
inline const BSDFSampleResult BSDF_SAMPLE_RESULT_INVALID =
    BSDFSampleResult({}, {}, 0, false);

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
    virtual FSpectrum eval_all(
        const FVec3 &wi, const FVec3 &wo, TransMode mode) const noexcept;
    /**
     * @brief eval f(in, out) or f*(in, out)
     */
    virtual FSpectrum eval(
        const FVec3 &wi, const FVec3 &wo,
        TransMode mode, uint8_t type) const noexcept = 0;

    /**
     * @brief given wo, sample wi
     */
    virtual BSDFSampleResult sample_all(
        const FVec3 &wo, TransMode mode, const Sample3 &sam) const noexcept;

    /**
     * @brief given wo, sample wi
     */
    virtual BSDFSampleResult sample(
        const FVec3 &wo, TransMode mode,
        const Sample3 &sam, uint8_t type) const noexcept = 0;

    /**
     * @brief pdf of sample
     */
    virtual real pdf(
        const FVec3 &wi, const FVec3 &wo,
        uint8_t type) const noexcept = 0;

    /**
     * @brief pdf of sample
     */
    virtual real pdf_all(const FVec3 &wi, const FVec3 &wo) const noexcept;

    /**
     * @brief material albedo
     */
    virtual FSpectrum albedo() const noexcept = 0;

    /**
     * @brief is this a delta function?
     */
    virtual bool is_delta() const noexcept = 0;

    /**
     * @brief does this bsdf contains a diffuse component?
     */
    virtual bool has_diffuse_component() const noexcept = 0;
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

    bool cause_black_fringes(const FVec3 &w1, const FVec3 &w2) const noexcept
    {
        return cause_black_fringes(w1) || cause_black_fringes(w2);
    }

    FSpectrum eval_black_fringes(const FVec3 &in, const FVec3 &out) const noexcept
    {
        if(!geometry_coord_.in_positive_z_hemisphere(in) ||
           !geometry_coord_.in_positive_z_hemisphere(out))
            return FSpectrum();
        return albedo() / PI_r;
    }

    BSDFSampleResult sample_black_fringes(
        const FVec3 &out, TransMode mode, const Sample3 &sam) const noexcept
    {
        if(!geometry_coord_.in_positive_z_hemisphere(out))
            return BSDF_SAMPLE_RESULT_INVALID;

        const auto [lwi, pdf] = math::distribution::
                                    zweighted_on_hemisphere(sam.u, sam.v);
        if(pdf < EPS())
            return BSDF_SAMPLE_RESULT_INVALID;

        const FVec3 wi = geometry_coord_.local_to_global(lwi).normalize();
        const real normal_corr = local_angle::normal_corr_factor(
            geometry_coord_, shading_coord_, wi);;
        const FSpectrum f = albedo() / PI_r * normal_corr;

        return BSDFSampleResult(wi, f, pdf, false);
    }

    real pdf_for_black_fringes(const FVec3 &in, const FVec3 &out) const noexcept
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
    LocalBSDF(const FCoord &geometry_coord, const FCoord &shading_coord) noexcept
        : geometry_coord_(geometry_coord), shading_coord_(shading_coord)
    {

    }
};

inline FSpectrum BSDF::eval_all(
    const FVec3 &wi, const FVec3 &wo, TransMode mode) const noexcept
{
    return eval(wi, wo, mode, BSDF_ALL);
}

inline BSDFSampleResult BSDF::sample_all(
    const FVec3 &wo, TransMode mode, const Sample3 &sam) const noexcept
{
    return sample(wo, mode, sam, BSDF_ALL);
}

inline real BSDF::pdf_all(const FVec3 &wi, const FVec3 &wo) const noexcept
{
    return pdf(wi, wo, BSDF_ALL);
}

AGZ_TRACER_END
