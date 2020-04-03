#include <agz/editor/material/material_thumbnail.h>
#include <agz/utility/image.h>

AGZ_EDITOR_BEGIN

namespace
{
    class MaterialThumbnailEnvLight
    {
        RC<const tracer::EnvirLight> env_light_;

    public:

        MaterialThumbnailEnvLight()
        {
            const unsigned char data[] = {
#include "./material_thumbnail_env.txt"
            };

            auto tex_data = img::load_rgb_from_hdr_memory(data, sizeof(data));
            auto img_data = newRC<Image2D<math::color3f>>(std::move(tex_data));
            auto tex = tracer::create_hdr_texture({}, img_data, "linear");
            env_light_ = create_ibl_light(tex);
        }

        const tracer::EnvirLight *operator->() const
        {
            return env_light_.get();
        }
    };

    Spectrum illum(
        const Vec3 &wo, const Vec3 &nor, const tracer::BSDF *bsdf,
        const MaterialThumbnailEnvLight &light,
        tracer::Sampler &sampler)
    {
        Spectrum bsdf_illum, light_illum;

        const auto bsdf_sample = bsdf->sample_all(
            wo, tracer::TransMode::Radiance, sampler.sample3());
        if(!bsdf_sample.f.is_black())
        {
            const real cos_v = std::abs(cos(bsdf_sample.dir, nor));
            const Spectrum env = light->radiance({}, bsdf_sample.dir);

            if(bsdf->is_delta())
                bsdf_illum = bsdf_sample.f * cos_v * env / bsdf_sample.pdf;
            else
            {
                const real env_pdf = light->pdf({}, bsdf_sample.dir);
                bsdf_illum = bsdf_sample.f * cos_v * env
                           / (bsdf_sample.pdf + env_pdf);
            }
        }

        const auto light_sample = light->sample({}, sampler.sample5());
        if(!light_sample.radiance.is_black())
        {
            const Vec3 wi = light_sample.ref_to_light();
            const Spectrum f = bsdf->eval_all(wi, wo, tracer::TransMode::Radiance);
            const real cos_v = std::abs(cos(wi, nor));
            const real bsdf_pdf = bsdf->pdf_all(wi, wo);

            light_illum = light_sample.radiance * f * cos_v
                        / (light_sample.pdf + bsdf_pdf);
        }

        return bsdf_illum + light_illum;
    }
}

// IMPROVE: bssrdf is not handled
void MaterialThumbnailProvider::run_one_iter(int spp)
{
    static const MaterialThumbnailEnvLight env;
    tracer::Arena arena;

    real init_xf, zf, df;
    if(width_ < height_)
    {
        df = real(6) / width_;
        init_xf = -3 + df / 2;
        zf = real(height_) / width_ * 3 - df / 2;
    }
    else
    {
        df = real(6) / height_;
        zf = 3 - df / 2;
        init_xf = -real(width_) / height_ * 3 + df / 2;
    }

    for(int y = 0; y < height_; ++y)
    {
        real xf = init_xf;

        if(exit_)
            return;

        for(int x = 0; x < width_; ++x)
        {
            for(int s = 0; s < spp; ++s)
            {
                const real pxf = xf + (sampler_->sample1().u - 1) * df;
                const real pzf = zf + (sampler_->sample1().u - 1) * df;

                if(pxf * pxf + pzf * pzf < 4)
                {
                    const real pyf = -std::sqrt(4 - pxf * pxf - pzf * pzf);

                    tracer::Coord coord; Vec2 uv;
                    tracer::sphere::local_geometry_uv_and_coord(
                        { pxf, pyf, pzf }, &uv, &coord, 2);

                    tracer::SurfacePoint spt;
                    spt.pos = { pxf, pyf, pzf };
                    spt.uv = uv;
                    spt.geometry_coord = coord;
                    spt.user_coord = coord;

                    tracer::EntityIntersection inct;
                    (tracer::SurfacePoint &)inct = spt;
                    inct.entity     = nullptr;
                    inct.material   = mat_.get();
                    inct.medium_in  = nullptr;
                    inct.medium_out = nullptr;
                    inct.wr         = { 0, 0, 1 };
                    inct.t          = 1;

                    auto shd = mat_->shade(inct, arena);

                    Spectrum color;
                    for(int j = 0; j < 4; ++j)
                    {
                        color += illum(
                            { 0, -1, 0 }, spt.geometry_coord.z,
                            shd.bsdf, env, *sampler_);
                    }
                    accum_color_(y, x) += real(0.25) * color;
                }
                else
                    accum_color_(y, x) += Spectrum(real(0.2));
            }

            xf += df;
        }

        zf -= df;
    }

    ++finished_iters_;
    finished_spp_ += spp;
}

QPixmap MaterialThumbnailProvider::compute_pixmap()
{
    assert(finished_iters_ > 0);
    const real ratio = real(1) / finished_spp_;

    QImage img(width_, height_, QImage::Format_RGB888);
    for(int y = 0; y < height_; ++y)
    {
        for(int x = 0; x < width_; ++x)
        {
            const Spectrum color = accum_color_(y, x).map([=](real c)
            {
                return std::pow(c * ratio, 1 / real(2.2));
            }).saturate();

            img.setPixelColor(x, y, QColor::fromRgbF(color.r, color.g, color.b));
        }
    }

    QPixmap ret;
    ret.convertFromImage(img);
    return ret;
}

MaterialThumbnailProvider::MaterialThumbnailProvider(
    int width, int height, RC<const tracer::Material> mat,
    int iter_spp, int iter_count)
    : width_(width), height_(height), mat_(std::move(mat)), exit_(false)
{
    iter_spp_ = iter_spp;
    iter_count_ = iter_count;

    finished_iters_ = 0;
    finished_spp_   = 0;
}

MaterialThumbnailProvider::~MaterialThumbnailProvider()
{
    assert(!exit_);
    exit_ = true;
    if(render_thread_.joinable())
        render_thread_.join();
}

QPixmap MaterialThumbnailProvider::start()
{
    assert(!exit_);

    accum_color_.initialize(height_, width_, Spectrum());
    sampler_ = newRC<tracer::NativeSampler>(42, false);

    run_one_iter(1);
    auto ret = compute_pixmap();

    render_thread_ = std::thread([=]
    {
        for(;;)
        {
            if(exit_)
                return;

            if(finished_iters_ >= iter_count_)
                return;

            run_one_iter(iter_spp_);

            if(exit_)
                return;

            emit update_thumbnail(compute_pixmap());
        }
    });

    return ret;
}

AGZ_EDITOR_END
