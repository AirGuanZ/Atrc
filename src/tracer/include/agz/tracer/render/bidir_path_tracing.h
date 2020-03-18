#pragma once

#include <agz/tracer/core/render_target.h>
#include <agz/tracer/render/common.h>

AGZ_TRACER_RENDER_BEGIN

namespace bdpt
{

// =========================== bdpt operations ===========================

/*
 * @brief bdpt path vertex
 *
 * [e] means this field is useful only when entity != nullptr
 */
struct BDPTVertex
{
    // vertex position when entity != nullptr
    // ray.d when entity == nullptr
    Vec3 pos;

    // vertex normal[e]
    Vec3 nor;

    // accumulated bsdf from end point to this vertex
    // including camera we and light le
    Spectrum accu_bsdf;

    // accumulated proj pdf from end point to this vertex
    // including starting pdf_pos
    real accu_proj_pdf = 0;

    // proj pdf of sampling this vertex along camera -> light direction
    // camera end point.pdf_fwd stores pdf_pos
    real pdf_fwd = 0;

    // proj pdf of sampling this vertex along light -> camera direction
    // light end point.pdf_bwd stores pdf_pos
    real pdf_bwd = 0;

    // G between last vertex[e]
    real G_with_last = 0;

    // vertex entity
    const Entity *entity = nullptr;

    // vertex uv[e]
    Vec2 uv;

    // vertex bsdf[e]
    const BSDF *bsdf = nullptr;

    // is this a delta vertex
    bool is_delta = false;

    // is this a entity vertex
    // meaningless to camera/light end point
    bool is_entity() const noexcept { return entity != nullptr; }
};

/**
 * @brief return value of build_camera_subpath
 */
struct CameraSubpath
{
    real pixel_x = 0;
    real pixel_y = 0;

    int vtx_cnt = 0;
    BDPTVertex *subpath = nullptr;

    struct GPixel
    {
        Spectrum albedo;
        Vec3 normal;
        real denoise = 1;
    } gpixel;
};

/**
 * @brief return value of build_light_subpath
 */
struct LightSubpath
{
    int vtx_cnt = 0;
    BDPTVertex *subpath = nullptr;

    real select_light_pdf = 0;
    const Light *light = nullptr;
};

/**
 * @brief result of connect two end points of camera/light subpath
 *
 * @note bdpt in atrc doesn't support sampling tech with s == 0
 *  (eval inct with camera lens), thus assert(s >= 1)
 */
struct ConnectedPath
{
    const Scene &scene;
    const Light *light;
    const Camera *camera;

    real select_light_pdf = 0;

    BDPTVertex *cam_subpath; int s;
    BDPTVertex *lht_subpath; int t;

    Sampler &sampler;

    Rect2 sample_pixel_bound;
    const Vec2 full_res;
};

CameraSubpath build_camera_subpath(
    int max_cam_vtx_cnt,
    int px, int py, const Scene &scene, const Vec2 &full_res,
    Sampler &sampler, Arena &arena, BDPTVertex *subpath_space);

LightSubpath build_light_subpath(
    int max_lht_vtx_cnt, const Scene &scene,
    Sampler &sampler, Arena &arena, BDPTVertex *subpath_space);

/**
 * pixel_coord shall not be nullptr iff connected_path.s == 1
 */
Spectrum eval_connected_subpath(
    const ConnectedPath &connected_path, Vec2 *pixel_coord);

// =========================== final bdpt ===========================

struct BDPTParams
{
    int max_cam_vtx_cnt = 10;
    int max_lht_vtx_cnt = 10;
};

struct BDPTPixel
{
    real px, py;
    Pixel pixel;
};

/**
 * @brief trace a bdpt path
 *
 * size of cam_subpath_space must be ge than params.max_cam_vtx_cnt
 * size of lht_subpath_space must be ge than params.max_lht_vtx_cnt
 */
std::optional<BDPTPixel> trace_bdpt(
    const BDPTParams &params, const Scene &scene,
    int px, int py, const Vec2 &full_res,
    Sampler &sampler, Arena &arena,
    BDPTVertex *cam_subpath_space,
    BDPTVertex *lht_subpath_space,
    FilmFilterApplier::FilmGridView<Spectrum> *particle_film);

} // namespace bdpt

AGZ_TRACER_RENDER_END
