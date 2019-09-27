#pragma once

#include <agz/tracer/core/object.h>

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

    /**
     * @brief 摄像机在world space中的bounding box
     */
    virtual AABB world_bound() const noexcept = 0;

    /**
     * @brief 生成一条射线
     */
    virtual CameraGenerateRayResult generate_ray(const CameraSample &sam) const noexcept = 0;
};

AGZT_INTERFACE(Camera)

AGZ_TRACER_END
