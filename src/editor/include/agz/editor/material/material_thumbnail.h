#pragma once

#include <agz/editor/material/material.h>

AGZ_EDITOR_BEGIN

class MaterialThumbnailProvider : public ResourceThumbnailProvider
{
    int width_;
    int height_;
    std::shared_ptr<const tracer::Material> mat_;

    std::atomic<bool> exit_;
    std::thread render_thread_;

    int iter_spp_;
    int iter_count_;

    int finished_iters_;
    int finished_spp_;
    Image2D<Spectrum> accum_color_;
    std::shared_ptr<tracer::Sampler> sampler_;

    void run_one_iter(int spp);

    QPixmap compute_pixmap();

public:

    MaterialThumbnailProvider(
        int width, int height, std::shared_ptr<const tracer::Material> mat, int iter_spp = 16, int iter_count = 10);

    ~MaterialThumbnailProvider();

    QPixmap start() override;
};

AGZ_EDITOR_END
