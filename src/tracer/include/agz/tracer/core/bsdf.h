#pragma once

#include <agz/tracer/common.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

/**
 * @brief what is carried by the path: radiance/importance
 */
enum class TransportMode
{
    Radiance   = 0, // camera -> light
    Importance = 1  // light -> camera
};

/**
 * @brief result of sampling BSDF
 */
struct BSDFSampleResult
{
    Vec3          dir;              // scattering direction
    Spectrum      f;                // bsdf value
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
    { {}, {}, 0, false };

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
    virtual Spectrum eval(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept = 0;

    /**
     * @brief given wo, sample wi
     */
    virtual BSDFSampleResult sample(const Vec3 &wo, TransportMode mode, const Sample3 &sam) const noexcept = 0;

    /**
     * @brief pdf of sample
     */
    virtual real pdf(const Vec3 &wi, const Vec3 &wo) const noexcept = 0;

    /**
     * @brief material albedo
     */
    virtual Spectrum albedo() const noexcept = 0;

    /**
     * @brief is this a delta function?
     */
    virtual bool is_delta() const noexcept = 0;
};

/**
 * @brief BSDF with local frame
 */
class LocalBSDF : public BSDF
{
protected:

    Coord geometry_coord_;
    Coord shading_coord_;

    // black fringes: cases when w is in different hemispheres of geometry_coord and shading_coord.
    // in these cases, BSDF values are undefined
    // following 5 methods are used for handling black bringes quickly

    bool cause_black_fringes(const Vec3 &w) const noexcept
    {
        const bool shading_posi  = shading_coord_.in_positive_z_hemisphere(w);
        const bool geometry_posi = geometry_coord_.in_positive_z_hemisphere(w);
        return shading_posi != geometry_posi;
    }

    bool cause_black_fringes(const Vec3 &w1, const Vec3 &w2) const noexcept
    {
        return cause_black_fringes(w1) || cause_black_fringes(w2);
    }

    Spectrum eval_for_black_fringes(const Vec3 &in, const Vec3 &out) const noexcept
    {
        if(!geometry_coord_.in_positive_z_hemisphere(in) ||
           !geometry_coord_.in_positive_z_hemisphere(out))
            return Spectrum();
        return albedo() / PI_r;
    }

    BSDFSampleResult sample_for_black_fringes(const Vec3 &out, TransportMode mode, const Sample3 &sam) const noexcept
    {
        if(!geometry_coord_.in_positive_z_hemisphere(out))
            return BSDF_SAMPLE_RESULT_INVALID;

        const auto [local_in, pdf] = math::distribution::zweighted_on_hemisphere(sam.u, sam.v);
        if(pdf < EPS)
            return BSDF_SAMPLE_RESULT_INVALID;

        BSDFSampleResult ret;
        ret.dir      = geometry_coord_.local_to_global(local_in).normalize();
        ret.f        = albedo() / PI_r * local_angle::normal_corr_factor(geometry_coord_, shading_coord_, ret.dir);
        ret.pdf      = pdf;
        ret.is_delta = false;

        return ret;
    }

    real pdf_for_black_fringes(const Vec3 &in, const Vec3 &out) const noexcept
    {
        if(geometry_coord_.in_positive_z_hemisphere(in) != geometry_coord_.in_positive_z_hemisphere(out))
            return false;
        const Vec3 local_in = geometry_coord_.global_to_local(in).normalize();
        return math::distribution::zweighted_on_hemisphere_pdf(local_in.z);
    }

public:

    /**
     * @param geometry_coord geometric local frame, determined by the mathematically correct geometric properties of the object
     * @param shading_coord shading local frame, constructed by geometry_coord, normal interpolation and normal mapping
     */
    LocalBSDF(const Coord &geometry_coord, const Coord &shading_coord) noexcept
        : geometry_coord_(geometry_coord), shading_coord_(shading_coord)
    {

    }
};

AGZ_TRACER_END
