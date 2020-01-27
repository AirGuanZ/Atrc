#pragma once

#include <any>

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

/**
 * @brief 摄像机采样射线的结果
 */
struct CameraSampleWeResult
{
    Vec3 pos_on_cam;
    Vec3 pos_to_out;
    Vec3 nor_on_cam;
    Spectrum throughput;
};

/**
 * @brief 摄像机求sample pdf的结果
 */
struct CameraWePDFResult
{
    real pdf_pos      = 0;
    real pdf_dir      = 0;
};

/**
 * @brief 摄像机eval initial importance function的结果
 */
struct CameraEvalWeResult
{
    Spectrum we;
    Vec2 film_coord;
};

inline const CameraEvalWeResult CAMERA_EVAL_WE_RESULT_ZERO = { {}, {} };

/**
 * @brief 摄像机sample we的结果
 */
struct CameraSampleWiResult
{
    Vec3 pos_on_cam; // 镜头上的点的位置
    Vec3 nor_at_pos; // 镜头法线
    Vec3 ref_to_pos; // 从ref到镜头点的方向
    Spectrum we;     // initial importance function
    real pdf = 0;    // pdf w.r.t. solid angle at ref
    Vec2 film_coord; // 对应于film coord上的何处
};

inline const CameraSampleWiResult CAMERA_SAMPLE_WI_RESULT_INVALID =
    { {}, {}, {}, {}, 0, {} };

/**
 * @brief 摄像机接口
 * 
 * 支持以下操作：
 * - 给定film坐标，生成一条radiance ray，带有We值和pdf
 * - 给定ref点，给一条从ref点到camera的采样射线，附带We值和pdf
 */
class Camera
{
protected:

    int film_width_;
    int film_height_;

public:

    explicit Camera(int film_width, int film_height) noexcept : film_width_(film_width), film_height_(film_height) { }

    virtual ~Camera() = default;

    virtual void update_param(std::string_view name, const std::any &value) = 0;

    /**
     * @brief 生成一条射线
     *
     * @param film_coord film上的坐标，以左下角为原点，右为+x上为+y，坐标范围[0, 1]^2
     * @param aperture_sample 用于采样镜头的sample点
     */
    virtual CameraSampleWeResult sample_we(
        const Vec2 &film_coord, const Sample2 &aperture_sample) const noexcept = 0;

    /**
     * @brief 求we值
     *
     * @param pos_on_cam 镜头上的点的位置
     * @param pos_to_out 从镜头指向外部的方向
     */
    virtual CameraEvalWeResult eval_we(const Vec3 &pos_on_cam, const Vec3 &pos_to_out) const noexcept = 0;

    /**
     * @brief 求sample_we的pdf
     */
    virtual CameraWePDFResult pdf_we(const Vec3 &pos_on_cam, const Vec3 &pos_to_out) const noexcept = 0;

    /**
     * @brief 采样从ref到镜头上某点的方向
     */
    virtual CameraSampleWiResult sample_wi(const Vec3 &ref, const Sample2 &sam) const noexcept = 0;

    /**
     * @brief 取得bounding box
     */
    virtual AABB get_world_bound() const noexcept = 0;
};

AGZ_TRACER_END
