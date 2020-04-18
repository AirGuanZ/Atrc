#pragma once

#include <thread>

#include <agz/editor/material/material.h>

AGZ_EDITOR_BEGIN

/**
 * @brief refine a thumbnail progressively using monte-carlo method
 */
class MaterialThumbnailProvider : public ResourceThumbnailProvider
{
    int width_;
    int height_;
    RC<const tracer::Material> mat_;

    std::atomic<bool> exit_;
    std::thread render_thread_;

    int iter_spp_;
    int iter_count_;

    int finished_iters_;
    int finished_spp_;
    Image2D<Spectrum> accum_color_;
    RC<tracer::Sampler> sampler_;

    void run_one_iter(int spp);

    QPixmap compute_pixmap();

public:

    /**
     * @param iter_spp spp per iteration
     * @param iter_count number of iteration
     *
     * the first iteration uses only 1 spp and provides the initial thumbnail
     *  (returned by start())
     */
    MaterialThumbnailProvider(
        int width, int height, RC<const tracer::Material> mat,
        int iter_spp = 8, int iter_count = 16);

    ~MaterialThumbnailProvider();

    QPixmap start() override;
};

AGZ_EDITOR_END
