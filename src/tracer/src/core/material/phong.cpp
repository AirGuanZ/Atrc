#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture2d.h>

AGZ_TRACER_BEGIN

namespace phong_impl
{

    class PhongBSDF : public LocalBSDF
    {
        Spectrum d_;
        Spectrum s_;
        real ns_;

        real diffuse_pdf_;
        real specular_pdf_;

        Vec3 sample_pow_cos_on_hemisphere(
            real e, const Sample2 &sam) const noexcept
        {
            const real cos_theta_h = std::pow(sam.u, 1 / (e + 1));
            const real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
            const real phi = 2 * PI_r * sam.v;

            return Vec3(
                sin_theta_h * std::cos(phi),
                sin_theta_h * std::sin(phi),
                cos_theta_h).normalize();
        }

        real pow_cos_on_hemisphere_pdf(real e, real cos_theta) const noexcept
        {
            return (e + 1) / (2 * PI_r) * std::pow(cos_theta, e);
        }

        Spectrum eval_diffuse(const Vec3 &lwi, const Vec3 &lwo) const noexcept
        {
            assert(lwi.z > 0 && lwo.z > 0);
            return d_ / PI_r;
        }

        Spectrum eval_specular(const Vec3 &lwi, const Vec3 &lwo) const noexcept
        {
            assert(lwi.z > 0 && lwo.z > 0);

            const Vec3 wh = (lwi + lwo).normalize();
            const real D = pow_cos_on_hemisphere_pdf(ns_, wh.z);

            return s_ * D / (4 * lwi.z * lwo.z);
        }

        std::pair<Vec3, real> sample_diffuse(
            const Vec3 &lwo, const Sample2 &sam) const noexcept
        {
            assert(lwo.z > 0);
            return math::distribution::zweighted_on_hemisphere(sam.u, sam.v);
        }

        std::pair<Vec3, real> sample_specular(
            const Vec3 &lwo, const Sample2 &sam) const noexcept
        {
            assert(lwo.z > 0);

            const Vec3 wh = sample_pow_cos_on_hemisphere(ns_, sam);
            const Vec3 lwi = (2 * dot(lwo, wh) * wh - lwo).normalize();
            if(lwi.z <= 0)
                return { {}, 0 };

            const real D = pow_cos_on_hemisphere_pdf(ns_, wh.z);
            const real pdf = D / (4 * dot(lwo, wh));

            return { lwi, pdf };
        }

        real pdf_diffuse(const Vec3 &lwi, const Vec3 &lwo) const noexcept
        {
            assert(lwi.z > 0 && lwo.z > 0);
            return math::distribution::zweighted_on_hemisphere_pdf(lwi.z);
        }

        real pdf_specular(const Vec3 &lwi, const Vec3 &lwo) const noexcept
        {
            assert(lwi.z > 0 && lwo.z > 0);
            const Vec3 wh = (lwi + lwo).normalize();
            return pow_cos_on_hemisphere_pdf(ns_, wh.z) / (4 * dot(lwo, wh));
        }

    public:

        PhongBSDF(
            const Coord &geometry_coord, const Coord &shading_coord,
            const Spectrum &d, const Spectrum &s, real ns)
            : LocalBSDF(geometry_coord, shading_coord),
              d_(d), s_(s), ns_(ns)
        {
            const real diffuse_lum  = d_.lum();
            const real specular_lum = s_.lum();
            if(diffuse_lum + specular_lum > EPS)
                diffuse_pdf_ = diffuse_lum / (diffuse_lum + specular_lum);
            else
                diffuse_pdf_ = real(0.5);

            specular_pdf_ = 1 - diffuse_pdf_;
        }

        Spectrum eval(
            const Vec3 &wi, const Vec3 &wo,
            TransMode mode) const noexcept override
        {
            if(cause_black_fringes(wi, wo))
                return eval_black_fringes(wi, wo);

            const Vec3 lwi = shading_coord_.global_to_local(wi);
            const Vec3 lwo = shading_coord_.global_to_local(wo);
            if(lwi.z <= 0 || lwo.z <= 0)
                return Spectrum();

            const Spectrum d = eval_diffuse(lwi, lwo);
            const Spectrum s = eval_specular(lwi, lwo);

            const real nor_corr = local_angle::normal_corr_factor(
                    geometry_coord_, shading_coord_, wi);

            return (d + s) * nor_corr;
        }

        BSDFSampleResult sample(
            const Vec3 &wo, TransMode mode, 
            const Sample3 &sam) const noexcept override
        {
            if(cause_black_fringes(wo))
                return sample_black_fringes(wo, mode, sam);

            const Vec3 &lwo = shading_coord_.global_to_local(wo);
            if(lwo.z <= 0)
                return BSDF_SAMPLE_RESULT_INVALID;

            if(sam.u < diffuse_pdf_)
            {
                auto [lwi, pdf_d] = sample_diffuse(lwo, { sam.v, sam.w });

                const Vec3 wi = shading_coord_.local_to_global(lwi);
                const real nor_corr = local_angle::normal_corr_factor(
                    geometry_coord_, shading_coord_, wi);

                BSDFSampleResult ret;
                ret.f   = (eval_diffuse(lwi, lwo) + eval_specular(lwi, lwo)) * nor_corr;
                ret.dir = wi;
                ret.pdf = diffuse_pdf_ * pdf_d + specular_pdf_ * pdf_specular(lwi, lwo);
                ret.is_delta = false;

                return ret;
            }

            auto [lwi, pdf_s] = sample_specular(lwo, { sam.v, sam.w });
            if(!pdf_s)
                return BSDF_SAMPLE_RESULT_INVALID;

            const Vec3 wi = shading_coord_.local_to_global(lwi);
            const real nor_corr = local_angle::normal_corr_factor(
                geometry_coord_, shading_coord_, wi);

            BSDFSampleResult ret;
            ret.f   = (eval_diffuse(lwi, lwo) + eval_specular(lwi, lwo)) * nor_corr;
            ret.dir = wi;
            ret.pdf = diffuse_pdf_ * pdf_diffuse(lwi, lwo) + specular_pdf_ * pdf_s;
            ret.is_delta = false;

            if(!ret.f.is_finite() || ret.pdf < EPS)
                return BSDF_SAMPLE_RESULT_INVALID;

            return ret;
        }

        real pdf(const Vec3 &wi, const Vec3 &wo) const noexcept override
        {
            if(cause_black_fringes(wi, wo))
                return pdf_for_black_fringes(wi, wo);

            const Vec3 lwi = shading_coord_.global_to_local(wi);
            const Vec3 lwo = shading_coord_.global_to_local(wo);
            if(lwi.z <= 0 || lwo.z <= 0)
                return 0;

            return diffuse_pdf_ * pdf_diffuse(lwi, lwo) +
                   specular_pdf_ * pdf_specular(lwi, lwo);
        }

        Spectrum albedo() const noexcept override
        {
            return d_ + s_;
        }

        bool is_delta() const noexcept override
        {
            return false;
        }

        bool has_diffuse_component() const noexcept override
        {
            return !d_.is_black();
        }
    };

} // namespace phong_impl

class Phong : public Material
{
    RC<const Texture2D> d_;
    RC<const Texture2D> s_;
    RC<const Texture2D> ns_;

    Box<NormalMapper> nor_map_;

public:

    Phong(
        RC<const Texture2D> d,
        RC<const Texture2D> s,
        RC<const Texture2D> ns,
        Box<NormalMapper> nor_map)
        : d_(std::move(d)), s_(std::move(s)),
          ns_(std::move(ns)), nor_map_(std::move(nor_map))
    {
        
    }

    ShadingPoint shade(const SurfacePoint &inct, Arena &arena) const override
    {
        Spectrum d = d_->sample_spectrum(inct.uv);
        Spectrum s = s_->sample_spectrum(inct.uv);
        const real ns = ns_->sample_real(inct.uv);

        real dem = 1;
        for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
            dem = (std::max)(dem, d[i] + s[i]);
        d /= dem;
        s /= dem;

        const Coord shading_coord = nor_map_->reorient(inct.uv, inct.user_coord);

        ShadingPoint shd;
        shd.bsdf = arena.create<phong_impl::PhongBSDF>(
            inct.geometry_coord, shading_coord, d, s, ns);
        shd.shading_normal = shading_coord.z;

        return shd;
    }
};

RC<Material> create_phong(
    RC<const Texture2D> d,
    RC<const Texture2D> s,
    RC<const Texture2D> ns,
    Box<NormalMapper> nor_map)
{
    return newRC<Phong>(
        std::move(d), std::move(s), std::move(ns), std::move(nor_map));
}

AGZ_TRACER_END
