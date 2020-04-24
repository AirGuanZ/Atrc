#pragma once

#include <any>

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

/**
 * @brief result of sampling we
 */
struct CameraSampleWeResult
{
    CameraSampleWeResult(
        const Vec3 &pos_on_cam,
        const Vec3 &pos_to_out,
        const Vec3 &nor_on_cam,
        const Spectrum &throughput) noexcept
        : pos_on_cam(pos_on_cam),
          pos_to_out(pos_to_out),
          nor_on_cam(nor_on_cam),
          throughput(throughput)
    {

    }

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
    CameraWePDFResult(real pdf_pos, real pdf_dir) noexcept
        : pdf_pos(pdf_pos), pdf_dir(pdf_dir)
    {
        
    }

    real pdf_pos;
    real pdf_dir;
};

/**
 * @brief result of eval we
 */
struct CameraEvalWeResult
{
    CameraEvalWeResult(
        const Spectrum &we,
        const Vec2 &film_coord,
        const Vec3 &nor_on_cam) noexcept
        : we(we), film_coord(film_coord), nor_on_cam(nor_on_cam)
    {

    }

    Spectrum we;
    Vec2 film_coord;
    Vec3 nor_on_cam;
};

inline const CameraEvalWeResult CAMERA_EVAL_WE_RESULT_ZERO =
    CameraEvalWeResult({}, {}, {});

/**
 * @brief result of sampling camera wi
 */
struct CameraSampleWiResult
{
    CameraSampleWiResult(
        const Vec3 &pos_on_cam,
        const Vec3 &nor_at_pos,
        const Vec3 &ref_to_pos,
        const Spectrum &we,
        real pdf,
        const Vec2 &film_coord) noexcept
        : pos_on_cam(pos_on_cam),
          nor_at_pos(nor_at_pos),
          ref_to_pos(ref_to_pos),
          we(we),
          pdf(pdf),
          film_coord(film_coord)
    {
        
    }

    Vec3 pos_on_cam; // position on camera lens
    Vec3 nor_at_pos; // lens normal
    Vec3 ref_to_pos; // from reference point to position on lens
    Spectrum we;     // initial importance function
    real pdf = 0;    // pdf w.r.t. solid angle at ref
    Vec2 film_coord; // where on the film does this sample correspond to 
};

inline const CameraSampleWiResult CAMERA_SAMPLE_WI_RESULT_INVALID =
    CameraSampleWiResult({}, {}, {}, {}, 0, {});

/**
 * @brief camera interface
 */
class Camera
{
public:

    virtual ~Camera() = default;

    /**
     * @brief generate a ray
     *
     * @param film_coord film coordinate. origin is at the left-bottom corner
     *  and the coordinate range is [0, 1]^2
     * @param aperture_sam used to sample the aperture
     */
    virtual CameraSampleWeResult sample_we(
        const Vec2 &film_coord, const Sample2 &aperture_sam) const noexcept = 0;

    /**
     * @brief eval we(pos_on_cam -> pos_to_out)
     */
    virtual CameraEvalWeResult eval_we(
        const Vec3 &pos_on_cam, const Vec3 &pos_to_out) const noexcept = 0;

    /**
     * @brief pdf of sample_we
     */
    virtual CameraWePDFResult pdf_we(
        const Vec3 &pos_on_cam, const Vec3 &pos_to_out) const noexcept = 0;

    /**
     * @brief sample camera wi
     */
    virtual CameraSampleWiResult sample_wi(
        const Vec3 &ref, const Sample2 &sam) const noexcept = 0;
};

AGZ_TRACER_END
