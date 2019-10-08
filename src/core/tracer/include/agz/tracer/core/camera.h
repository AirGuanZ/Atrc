#pragma once

#include <agz/tracer/common.h>

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
    Spectrum throughput;
};

/**
 * @brief 摄像机接口
 * 
 * 支持以下操作：
 * - 给定film坐标，生成一条radiance ray，带有We值和pdf
 * - 给定ref点，给一条从ref点到camera的采样射线，附带We值和pdf
 */
class Camera
{
public:

    virtual ~Camera() = default;

    /**
     * @brief 摄像机在world space中的bounding box
     */
    virtual AABB world_bound() const noexcept = 0;

    /**
     * @brief 生成一条射线
     */
    virtual CameraGenerateRayResult generate_ray(const CameraSample &sam) const noexcept = 0;
};

AGZ_TRACER_END
