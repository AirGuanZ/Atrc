#pragma once

#include <agz/tracer/render/common.h>

AGZ_TRACER_RENDER_BEGIN

namespace sppm
{

/**
 * standard sppm process contains 4 steps:
 *  1. trace camera rays to create records of visible points
 *  2. build range search ds
 *  3. trace photons to update density estimations of flux at visible points
 *  4. update params at each pixel, and go back to step 1
 */

/**
 * @brief pixel params
 */
struct Pixel
{
    // visible point
    struct VisiblePoint
    {
        Vec3 pos;
        Vec3 wr;

        Spectrum    coef;
        const BSDF *bsdf = nullptr;

        bool is_valid() const noexcept
        {
            return bsdf != nullptr;
        }
    };

    Pixel() = default;

    Pixel(const Pixel &p) noexcept
    {
        vp     = p.vp;
        radius = p.radius;

        for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
            phi[i] = p.phi[i].load();

        M            = p.M.load();
        N            = p.N;
        tau          = p.tau;
        direct_illum = p.direct_illum;
    }

    // current visible point

    VisiblePoint vp;

    // standard sppm pixel params

    real radius = real(0.1);

    std::atomic<real> phi[SPECTRUM_COMPONENT_COUNT] = { 0 };
    std::atomic<int>  M = 0;

    real N = 0;
    Spectrum tau;

    // direct illumination

    Spectrum direct_illum;
};

class VisiblePointSearcher
{
public:

    explicit VisiblePointSearcher(
        const AABB &world_bound,
        int max_axis_grid_res = 128);

    /**
     * @brief clear all vp records
     */
    void clear();

    /**
     * @brief add a vp record
     *
     * parallel 'add_vp' is safe
     */
    void add_vp(Pixel &pixel, Arena &vp_node_arena);

    /**
     * @brief accumulate photon flux to recorded visible points
     *
     * parallel 'add_vp' is safe
     */
    void add_photon(const Vec3 &photon_pos, const Spectrum &phi, const Vec3 &wr);

private:

    Vec3i pos_to_grid(const Vec3 &pos) const noexcept;

    int grid_to_entry(const Vec3i &grid_index) const noexcept;

    int pos_to_entry(const Vec3 &pos) const noexcept;

    struct VPNode
    {
        Pixel *pixel = nullptr;
        VPNode *next = nullptr;
    };

    Vec3i grid_index_dot_;
    Box<std::atomic<VPNode*>[]> node_entries_;
    int node_entry_count_;

    Vec3 world_low_;
    real grid_size_;

    Vec3i grid_res_;
    int total_grid_count_;
};

/**
 * @brief sample a visible point along ray
 *
 * @param max_fwd_depth max tracing depth
 * @param direct_illum_spv nSamples for computing direct illum at each vertex
 * @param gpixel output g-buffer pixel (can be nullptr)
 * @param direct_illum accumulated direct illumination
 *
 * @return !ret.is_valid() means no visible point is found
 */
Pixel::VisiblePoint tracer_vp(
    int max_fwd_depth, int direct_illum_spv,
    const Scene &scene, const Ray &r, const Spectrum &init_coef,
    Arena &arena, Sampler &sampler,
    GBufferPixel *gpixel, Spectrum &direct_illum);

/**
 * trace a photon from light source and
 * accumulate flux at visible points in vp_searcher
 */
void trace_photon(
    int min_depth, int max_depth, real cont_prob,
    VisiblePointSearcher &vp_searcher,
    const Scene &scene, Arena &arena, Sampler &sampler);

void update_pixel_params(real alpha, Pixel &pixel);

Spectrum compute_pixel_radiance(
    int direct_illum_N, uint64_t photon_N, const Pixel &pixel);

} // namespace sppm

AGZ_TRACER_RENDER_END
