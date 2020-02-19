#pragma once

#include <any>

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

/**
 * @brief result of sampling we
 */
struct CameraSampleWeResult
{
    Vec3 pos_on_cam;
    Vec3 pos_to_out;
    Vec3 nor_on_cam;
    Spectrum throughput;
};

/**
 * @brief pdf of sampling we
 */
struct CameraWePDFResult
{
    real pdf_pos      = 0;
    real pdf_dir      = 0;
};

/**
 * @brief result of eval we
 */
struct CameraEvalWeResult
{
    Spectrum we;
    Vec2 film_coord;
};

inline const CameraEvalWeResult CAMERA_EVAL_WE_RESULT_ZERO = { {}, {} };

/**
 * @brief result of sampling camera wi
 */
struct CameraSampleWiResult
{
    Vec3 pos_on_cam; // position on camera lens
    Vec3 nor_at_pos; // lens normal
    Vec3 ref_to_pos; // from reference point to position on lens
    Spectrum we;     // initial importance function
    real pdf = 0;    // pdf w.r.t. solid angle at ref
    Vec2 film_coord; // where on the film does this sample correspond to 
};

inline const CameraSampleWiResult CAMERA_SAMPLE_WI_RESULT_INVALID =
    { {}, {}, {}, {}, 0, {} };

/**
 * @brief camera interface
 */
class Camera
{
public:

    virtual ~Camera() = default;

    virtual void update_param(std::string_view name, const std::any &value) = 0;

    /**
     * @brief generate a ray
     *
     * @param film_coord film coordinate. origin is at the left-bottom corner and the coord range is [0, 1]^2
     * @param aperture_sample used to sample the aperture
     */
    virtual CameraSampleWeResult sample_we(
        const Vec2 &film_coord, const Sample2 &aperture_sample) const noexcept = 0;

    /**
     * @brief eval we(pos_on_cam -> pos_to_out)
     */
    virtual CameraEvalWeResult eval_we(const Vec3 &pos_on_cam, const Vec3 &pos_to_out) const noexcept = 0;

    /**
     * @brief pdf of sample_we
     */
    virtual CameraWePDFResult pdf_we(const Vec3 &pos_on_cam, const Vec3 &pos_to_out) const noexcept = 0;

    /**
     * @brief sample camera wi
     */
    virtual CameraSampleWiResult sample_wi(const Vec3 &ref, const Sample2 &sam) const noexcept = 0;
};

AGZ_TRACER_END
