#pragma once

#include <agz/tracer/core/intersection.h>
#include <agz/tracer_utility/object.h>

AGZ_TRACER_BEGIN

/**
 * @brief 用于让摄像机生成primary ray的sample
 */
struct CameraSample
{
    Sample2 film_coord;      // film上的坐标，以左下角为原点，右为+x上为+y，坐标范围[0, 1]^2
    Sample2 aperture_sample; // 用于采样镜头的sample点
};

/**
 * @brief 摄像机采样射线的结果
 */
struct CameraGenerateRayResult
{
    Ray r;
    real importance = 1;

    real pdf_pos;
    bool is_pos_delta;

    real pdf_dir;
    bool is_dir_delta;

    Vec3 nor;
};

/**
 * @brief particle tracing中由摄像机对light ray进行采样的返回结果
 */
struct CameraSampleRayResult
{
    Vec3 ref_to_cam;         // 参考点到摄像机的射线方向
    Vec3 film_pos;           // world space中的摄像机镜头上的点的位置
    Vec2 film_coord;         // 对应于film上的什么位置（整个film是[0, 1]^2）
    real importance = 1;     // We值
    real pdf        = 1;     // w.r.t. solid angle at ref
    bool is_delta   = false; // pdf是否是delta function

    real gen_pdf_pos = 1; // 由摄像机主动生成此射线时所对应的pos pdf
    real gen_pdf_dir = 1; // 由摄像机主动生成此射线时所对应的dir pdf

    Vec3 nor;

    bool is_invalid() const noexcept
    {
        return !ref_to_cam;
    }
};

inline const CameraSampleRayResult CAMERA_SAMPLE_RAY_RESULT_INVALID = { Vec3(), Vec3(), Vec2(), 0, 0, false };

/**
 * @brief 摄像机接口
 * 
 * 支持以下操作：
 * - 给定film坐标，生成一条radiance ray，带有We值和pdf
 * - 给定ref点，给一条从ref点到camera的采样射线，附带We值和pdf
 */
class Camera : public obj::Object
{
public:

    using Object::Object;

    virtual AABB world_bound() const noexcept = 0;

    /**
     * @brief 生成一条射线
     */
    virtual CameraGenerateRayResult generate_ray(const CameraSample &sam) const noexcept = 0;

    /**
     * @brief 采样一条外部某点到camera的射线
     */
    virtual CameraSampleRayResult sample(const Vec3 &ref_pos, const Sample2 &sam) const noexcept = 0;
};

AGZT_INTERFACE(Camera)

AGZ_TRACER_END
